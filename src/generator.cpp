/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "generator.h"

#include <fmt/core.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <cmath>
#include <memory>

#include "v8pp/module.hpp"

#include "core/delayed_exit.hpp"
#include "interpreter/abort_exception.hpp"
#include "interpreter/fast_forwarder.hpp"
#include "scripting.h"
#include "starcry/metrics.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/memory_usage.hpp"
#include "util/step_calculator.hpp"
#include "util/vector_logic.hpp"

namespace interpreter {

generator::generator(std::shared_ptr<metrics>& metrics,
                     std::shared_ptr<v8_wrapper>& context,
                     const generator_options& opts)
    : context(context),
      metrics_(metrics),
      initializer_(*this),
      bridges_(*this),
      scenes_(*this),
      sampler_(*this),
      positioner_(*this),
      interactor_(*this),
      instantiator_(*this),
      job_mapper_(*this),
      generator_opts(opts) {}

void generator::init(const std::string& filename,
                     std::optional<double> rand_seed,
                     bool preview,
                     bool caching,
                     std::optional<int> width,
                     std::optional<int> height,
                     std::optional<double> scale) {
  prctl(PR_SET_NAME, "native generator thread");
  filename_ = filename;
  job = std::make_shared<data::job>();
  job->frame_number = frame_number;

  initializer_.initialize_all(rand_seed, preview, width, height, scale);

  context->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    genctx = std::make_shared<generator_context>(val, 0);
  });

  scenes_.initialize();

  // reset random number generator
  rand_.set_seed(seed);

  // set_scene requires generator_context to be set
  scenes_.set_scene(0);

  // all objects added at this point can be blindly appended
  scenes_.append_instantiated_objects();
}

void generator::create_object_instances() {
  // This function is called whenever a scene is set. (once per scene)
  context->enter_object("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    // enter_objects creates a new isolate, using the old gives issues, so we'll recreate
    genctx = std::make_shared<generator_context>(val, scenes_.current());
    genctx->set_scene(scenes_.current());

    scenes_.switch_scene();

    instantiator_.instantiate_additional_objects_from_new_scene(genctx->scene_objects, 0);

    // since this is invoked directly after a scene change, and in the very beginning, make sure this state is part of
    // the instances "current" frame, or reverting (e.g., due to motion blur requirements) will discard all of this.
    scenes_.commit_scene_shapes();
  });
}

void generator::create_bookkeeping_for_script_objects(v8::Local<v8::Object> created_instance,
                                                      const data_staging::shape_t& created_shape,
                                                      int debug_level) {
  auto& i = genctx->i();

  // only do extra work for script objects
  if (i.str(created_instance, "type") != "script") {
    return;
  }

  // created shape namespace
  std::string created_shape_namespace;
  data_staging::attrs obj_attrs;
  double scale = 1.;
  meta_callback(created_shape, [&]<typename T>(T& shape) {
    created_shape_namespace = shape.meta_cref().namespace_name();
    obj_attrs = shape.attrs_cref();
    scale = shape.generic_cref().scale();
  });

  const auto unique_id = i.integer_number(created_instance, "unique_id");
  const auto id = i.str(created_instance, "id");
  const auto namespace_ = created_shape_namespace;
  logger(DEBUG) << std::string(debug_level, ' ') << "Bookkeeping for: " << id << " (namespace: " << namespace_ << ")"
                << std::endl;
  i.set_field(created_instance, "namespace", v8_str(i.get_context(), namespace_));
  const auto file = i.str(created_instance, "file");
  const auto specified_duration = i.has_field(created_instance, "duration")
                                      ? i.double_number(created_instance, "duration")
                                      : static_cast<double>(-1);

  // read the entire script from disk
  std::ifstream stream(file);
  std::istreambuf_iterator<char> begin(stream), end;
  // remove "_ =" part from script (which is a workaround to silence 'not in use' linting)
  if (*begin == '_') {
    while (*begin != '=') begin++;
    begin++;
  }

  static std::atomic<int> uidx = 0;
  int use_index = uidx++;
  // evaluate script into temporary variable
  const auto source = fmt::format("var tmp{} = {};", use_index, std::string(begin, end));
  context->run(source);
  auto tmp = i.get_global(fmt::format("tmp{}", use_index)).As<v8::Object>();

  // process scenes and make the scenes relative, initialize helper objs etc
  auto scenes = i.v8_array(tmp, "scenes");
  {
    auto duration = scenes_.get_duration(unique_id);
    std::vector<double> durations;
    for (size_t I = 0; I < scenes->Length(); I++) {
      auto current_scene = i.get_index(scenes, I);
      if (!current_scene->IsObject()) continue;
      auto sceneobj = current_scene.As<v8::Object>();
      duration += i.double_number(sceneobj, "duration");
      durations.push_back(i.double_number(sceneobj, "duration"));
    }
    std::for_each(durations.begin(), durations.end(), [&duration](double& n) {
      n /= duration;
    });
    scenes_.set_duration(unique_id, duration);
    scenes_.set_durations(unique_id, durations);
    scenes_.set_desired_duration(unique_id, specified_duration);
  }

  // make the scenes a property of the created instance (even though we probably won't need it for now)
  i.set_field(created_instance, "scenes", scenes);  // TODO: remove?

  // import all gradients from script
  auto gradients = i.v8_obj(tmp, "gradients");
  auto gradient_fields = gradients->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
  for (size_t k = 0; k < gradient_fields->Length(); k++) {
    auto gradient_src_id = i.get_index(gradient_fields, k);
    auto gradient_dst_id = namespace_ + i.str(gradient_fields, k);
    i.set_field(genctx->gradients, gradient_dst_id, i.get(gradients, gradient_src_id));
  }
  initializer_.init_gradients();

  // import all textures from script
  auto textures = i.v8_obj(tmp, "textures");
  if (textures->IsObject()) {
    auto texture_fields = textures->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < texture_fields->Length(); k++) {
      auto texture_src_id = i.get_index(texture_fields, k);
      auto texture_dst_id = namespace_ + i.str(texture_fields, k);
      i.set_field(genctx->textures, texture_dst_id, i.get(textures, texture_src_id));
    }
    initializer_.init_textures();
  }

  // import all object definitions from script
  auto objects = i.v8_obj(tmp, "objects");
  auto objects_fields = objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
  for (size_t k = 0; k < objects_fields->Length(); k++) {
    auto object_src_id = i.get_index(objects_fields, k);
    auto object_dst_id = namespace_ + i.str(objects_fields, k);
    auto object_src = i.get(objects, object_src_id);
    auto object_src_obj = object_src.As<v8::Object>();
    logger(DEBUG) << std::string(debug_level, ' ') << "Importing object " << i.str(objects_fields, k)
                  << " as: " << object_dst_id << ", from: " << fmt::format("tmp{}", use_index) << " and file: " << file
                  << " , points to: " << i.str(object_src_obj, "file") << std::endl;

    // also copy all the stuff from the object definition
    i.recursively_copy_object(created_instance, object_src_obj);

    // also copy attrs to these objects imported by script
    if (!i.has_field(object_src_obj, "attrs")) {
      i.set_field(object_src_obj, "attrs", v8::Object::New(i.get_isolate()));
    }
    auto attrs = i.get(object_src_obj, "attrs").As<v8::Object>();
    for (const auto& str : obj_attrs.get_strings()) {
      i.set_field(attrs, str.first, v8_str(i.get_context(), str.second));
    }
    for (const auto& num : obj_attrs.get_numbers()) {
      i.set_field(attrs, num.first, v8::Number::New(i.get_isolate(), num.second));
    }
    i.set_field(object_src_obj, "attrs", attrs);

    // also copy scale as recursive_scale to all objects, but combine/merge if it already has a value (by multiplying)
    double existing_recursive_scale = i.double_number(object_src_obj, "recursive_scale", 1.0);
    i.set_field(object_src_obj, "recursive_scale", v8::Number::New(i.get_isolate(), existing_recursive_scale * scale));

    i.set_field(genctx->objects, object_dst_id, object_src);
    auto val = i.get(objects, object_src_id);
    object_definitions_map[object_dst_id].Reset(i.get_isolate(), val.As<v8::Object>());
  }

  // make sure we start from the current 'global' time as an offset
  scenes_.scenesettings_objs[unique_id].parent_offset = scenes_.get_time(scenes_.scenesettings).time;

  // sub object starts at scene zero
  scenes_.set_scene_sub_object(unique_id);

  // recurse for each object in the "sub" scene
  auto current_scene = i.get_index(scenes, scenes_.scenesettings_objs[unique_id].current_scene_next);
  if (current_scene->IsObject()) {
    auto o = current_scene.As<v8::Object>();
    auto scene_objects = i.v8_array(o, "objects");
    // TODO: why is it needed to convert these v8::Local objects? They seem to be garbage collected otherwise during
    // execution.
    v8::Persistent<v8::Array> tmp;
    tmp.Reset(i.get_isolate(), scene_objects);
    instantiator_.instantiate_additional_objects_from_new_scene(tmp, debug_level + 1, &created_shape);
  }

  // clean-up temporary variable that referenced the entire imported script
  context->run(fmt::format("tmp{} = undefined;", use_index));
}

void generator::reset_seeds() {
  instantiator_.reset_seeds();
}

void generator::fast_forward(int frame_of_interest) {
  fast_forwarder(fast_ff, frame_of_interest, min_intermediates, max_intermediates, [&]() {
    generate_frame();
    metrics_->skip_job(job->job_number);
  });
}

bool generator::generate_frame() {
  return sampler_.sample(use_fps, [&](bool skip) {
    auto ret = _generate_frame();
    if (skip) job->job_number--;
    return ret;
  });
}

bool generator::_generate_frame() {
  delayed_exit de(10);
  try {
    job->shapes.clear();

    // job_number is incremented later, hence we do a +1 on the next line.
    metrics_->register_job(job->job_number + 1, job->frame_number, job->chunk, job->num_chunks);

    context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
      genctx = std::make_shared<generator_context>(val, scenes_.scenesettings.current_scene_next);
      auto& i = genctx->i();

      auto obj = val.As<v8::Object>();
      auto scenes = i.v8_array(obj, "scenes");
      auto video = i.v8_obj(obj, "video");
      if (!video->IsObject()) video = v8::Object::New(isolate);

      // auto objects = i.v8_array(obj, "objects");
      auto current_scene = i.get_index(scenes, scenes_.scenesettings.current_scene_next);
      if (!current_scene->IsObject()) return;

      stepper.reset();
      indexes.clear();
      attempt = 0;
      max_dist_found = std::numeric_limits<double>::max();
      scalesettings.reset();

      if (min_intermediates > 0.) {
        update_steps(min_intermediates);
      }

      static const auto max_attempts = 2;
      while (max_dist_found > tolerated_granularity && !stepper.frozen) {
        if (++attempt >= max_attempts) {
          stepper.freeze();
        }
        // logger(DEBUG) << "Generating frame [native] " << job->frame_number << " attempt " << attempt << std::endl;
        max_dist_found = 0;
        if (attempt > 1) {
          if (!settings_.motion_blur) break;
          revert_all_changes(i);
        }
        step_calculator sc(stepper.max_step);
        job->resize_for_num_steps(stepper.max_step);
        metrics_->set_steps(job->job_number + 1, attempt, stepper.max_step);

        stepper.rewind();
        bool detected_too_many_steps = false;
        while (stepper.has_next_step() && !detected_too_many_steps) {
          // logger(DEBUG) << "Stepper at step " << stepper.current_step << " out of " << stepper.max_step << std::endl;
          stepper.advance_step();

          interactor_.reset();

          scenes_.prepare_scene();

          create_new_mappings();  // object uid -> object ref

          positioner_.update_object_positions();  // velocity + time

          positioner_.update_rotations();  // rotate + absolute x,y

          interactor_.update_interactions();  // toroidal, collisions, gravity, dedupe

          update_object_distances();  // calculate distance and steps

          // above update functions could have triggered spawning of new objects
          insert_newly_created_objects();

          job_mapper_.convert_objects_to_render_job(sc, video);

          scenes_.commit_scene_shapes_intermediates();

          scalesettings.update();
          scenes_.update();

          if (job->shapes.size() != size_t(stepper.max_step)) {
            detected_too_many_steps = true;
          }
          metrics_->update_steps(job->job_number + 1, attempt, stepper.current_step);
        }
        if (!detected_too_many_steps) {                 // didn't bail out with break above
          if (stepper.max_step == max_intermediates) {  // config doesn't allow finer granularity any way, break.
            break;
          }
          if (stepper.max_step > max_intermediates) {
            logger(DEBUG) << "stepper.max_step > max_intermediates -> " << stepper.max_step << " > "
                          << max_intermediates << std::endl;
            throw abort_exception("stepper max exceeds max. intermediates");
          }
        }
      }

      if (!settings_.update_positions) {
        positioner_.revert_position_updates();
      }

      /* The next statement potentially invalidates our mappings, but usually this doesn't matter, if we're rendering
       * frame after frame, we start fresh the next frame anyway. However, in interactive mode, the client can send
       * additional requests, such as rendering a specific set of IDs, and certain code gets exercised that relies on
       * the mappings.
       * We will set a dirty flag, so that if a client wishes to use the mappings, it should check the dirty flag.
       * We can, after some refactoring, put a nice wrapper type around this stuff.
       */
      bool destroyed = scenes_.cleanup_destroyed_objects();
      if (destroyed) {
        mappings_dirty = true;
      }

      scenes_.commit_scene_shapes();

      scalesettings.commit();
      scenes_.commit();
      if (generator_opts.debug) {
        debug_print_next();
      }

      // cleanup for next iteration
      interactor_.reset();

      metrics_->update_steps(job->job_number + 1, attempt, stepper.current_step);
    });

    job->job_number++;
    job->frame_number++;

    metrics_->complete_job(job->job_number);

    v8::HeapStatistics hs;
    context->isolate->GetHeapStatistics(&hs);
    const auto total_usage = (double(getValue()) / 1024. / 1024.);
    const auto v8_usage = (hs.total_heap_size() / 1024. / 1024. / 1024.);
    logger(INFO) << "Memory usage: " << total_usage << " GB. "
                 << "V8 Heap: " << v8_usage << " GB. "
                 << "Other: " << (total_usage - v8_usage) << " GB." << std::endl;
  } catch (abort_exception& ex) {
    std::cout << "[caught] " << ex.what() << " (abort)" << std::endl;
    std::exit(0);
  } catch (std::exception& ex) {
    std::cout << "[caught] " << ex.what() << std::endl;
  }
  job->last_frame = job->frame_number == max_frames;
  return job->frame_number != max_frames;
}

void generator::revert_all_changes(v8_interact& i) {
  job->shapes.clear();
  indexes.clear();

  // reset next and intermediate instances
  scenes_.reset_scene_shapes_next();
  scenes_.reset_scene_shapes_intermediates();

  scalesettings.revert();
  scenes_.revert();
}

void generator::create_new_mappings() {
  next_instance_map.clear();
  intermediate_map.clear();
  for (auto& abstract_shape : scenes_.next_shapes_current_scene()) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      next_instance_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
  for (auto& abstract_shape : scenes_.intermediate_shapes_current_scene()) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      intermediate_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
  mappings_dirty = false;
}

void generator::insert_newly_created_objects() {
  auto& dest = scenes_.next_shapes_current_scene();
  auto& source = scenes_.instantiated_objects_current_scene();
  if (source.empty()) return;

  dest.reserve(dest.size() + source.size());

  auto handle = [&](auto& shape, const data_staging::meta& meta) {
    if (meta.parent_uid() == -1 /* no parent */) {
      dest.emplace_back(std::move(shape));
    } else {
      // calculate first where to insert instance in destination array
      const auto parent_uid = meta.parent_uid();
      size_t insert_offset = dest.size();
      int64_t found_level = 0;
      bool searching = false;
      for (size_t j = 0; j < dest.size(); j++) {
        auto& elem = dest[j];
        std::optional<std::reference_wrapper<const data_staging::meta>> elem_meta;
        meta_callback(elem, [&]<typename TP>(TP& shape) {
          elem_meta = shape.meta_cref();
        });
        const auto uid = elem_meta->get().unique_id();
        const auto level = elem_meta->get().level();
        if (searching && level <= found_level) {
          insert_offset = j;
          break;
        } else if (uid == parent_uid) {
          found_level = level;
          // assume at this point it's the element after this one
          insert_offset = j + 1;
          searching = true;
          // NOTE: we can early exit here to spawn new objects on top within their parent
          // We can make that feature configurable, or even add some z-index-like support
          // break;
        }
      }

      // reverse iterate over the destination array
      // note that we iterate from one beyond the size of the array, if size() = 3, [3] will potentially be out of
      // bounds
      for (size_t rev_index = dest.size(); rev_index; rev_index--) {
        // insert element where we calculated it should be
        if (rev_index == insert_offset) {
          if (insert_offset == dest.size()) {
            dest.emplace_back(std::move(shape));
          } else {
            dest[insert_offset] = std::move(shape);
          }
          // no need to process the rest of the array at this point
          break;
        }
        // for all elements move them down so that we create space for the new element
        if (rev_index > 0) {
          if (rev_index == dest.size()) {
            dest.emplace_back(std::move(dest[rev_index - 1]) /* element above */);
          } else {
            dest[rev_index] = std::move(dest[rev_index - 1]) /* element above */;
          }
        }
      }
    }
  };
  for (auto& abstract_shape : source) {
    meta_callback(abstract_shape, [&]<typename TP>(TP& shape) {
      handle(abstract_shape, shape.meta_cref());
    });
  }
  source.clear();
  create_new_mappings();
}

void generator::update_object_distances() {
  stepper.reset_current();
  const auto handle = [&](data_staging::shape_t& abstract_shape, data_staging::meta& meta) {
    const auto instance_uid = meta.unique_id();
    const auto find = intermediate_map.find(instance_uid);
    if (find == intermediate_map.end()) {
      return;
    }
    const double dist = get_max_travel_of_object(abstract_shape, find->second.get());
    if (dist > max_dist_found) {
      max_dist_found = dist;
    }
    const auto steps = update_steps(dist);

    static std::unordered_map<int64_t, int> recorded_steps;

    if (attempt == 1) {
      meta.set_distance(dist);
      meta.set_steps(steps);
      recorded_steps[instance_uid] = steps;
    } else if (attempt > 1) {
      meta.set_steps(recorded_steps[instance_uid]);
    }
  };
  for (auto& abstract_shape : scenes_.next_shapes_current_scene()) {
    meta_callback(abstract_shape, [&]<typename TP>(TP& shape) {
      handle(abstract_shape, shape.meta_ref());
    });
  }
}

void generator::update_time(data_staging::shape_t& instance,
                            const std::string& instance_id,
                            scene_settings& scenesettings) {
  auto& i = genctx->i();
  const auto time_settings = scenes_.get_time(scenesettings);
  const auto execute = [&](double scene_time) {
    if (const auto find = object_definitions_map.find(instance_id); find != object_definitions_map.end()) {
      const auto object_definition = v8::Local<v8::Object>::New(i.get_isolate(), find->second);
      const auto handle_time_for_shape = [&](auto& c, auto& object_bridge) {
        // TODO: check if the object has an "time" function, or we can just skip this entire thing
        c.meta_ref().set_time(scene_time);
        object_bridge->push_object(c);
        i.call_fun(object_definition,
                   object_bridge->instance(),
                   "time",
                   scene_time,
                   time_settings.elapsed,
                   scenesettings.current_scene_next,
                   time_settings.time);
        object_bridge->pop_object();
      };
      meta_visit(
          instance,
          [&](data_staging::circle& c) {
            handle_time_for_shape(c, bridges_.circle());
          },
          [&](data_staging::ellipse& e) {
            handle_time_for_shape(e, bridges_.ellipse());
          },
          [&](data_staging::line& l) {
            handle_time_for_shape(l, bridges_.line());
          },
          [&](data_staging::text& t) {
            handle_time_for_shape(t, bridges_.text());
          },
          [&](data_staging::script& s) {
            handle_time_for_shape(s, bridges_.script());
          });
    }
  };

  if (scenesettings.current_scene_next > scenesettings.current_scene_intermediate) {
    // Make sure we end previous scene at the very last frame in any case, even though we won't render it.
    // This may be necessary to finalize some calculations that work with "t" (time), i.e., for rotations.
    const auto bak = scenesettings.current_scene_next;
    scenesettings.current_scene_next = scenesettings.current_scene_intermediate;
    execute(1.0);
    scenesettings.current_scene_next = bak;
  }
  execute(time_settings.scene_time);
}

int generator::update_steps(double dist) {
  auto steps = round(std::max(1.0, fabs(dist) / tolerated_granularity));
  if (steps > max_intermediates) {
    steps = max_intermediates;
  }
  stepper.update(steps);
  return steps;
}

double generator::get_max_travel_of_object(data_staging::shape_t& shape_now, data_staging::shape_t& shape_prev) {
  vector2d xy_now;
  vector2d xy_prev;
  vector2d xy2_now;
  vector2d xy2_prev;
  bool compare_xy2 = false;

  double radius_now = 0;
  double radius_prev = 0;

  meta_visit(
      shape_now,
      [&xy_now, &radius_now](data_staging::circle& shape) {
        xy_now = shape.transitive_location_ref().position_cref();
        radius_now = shape.radius() + shape.radius_size();
      },
      [&xy_now](data_staging::ellipse& shape) {
        xy_now = shape.transitive_location_ref().position_cref();
        // ...
      },
      [&xy_now, &xy2_now, &compare_xy2](data_staging::line& shape) {
        xy_now = shape.transitive_line_start_ref().position_cref();
        xy2_now = shape.transitive_line_end_ref().position_cref();
        compare_xy2 = true;
      },
      [&xy_now](data_staging::text& shape) {
        xy_now = shape.transitive_location_ref().position_cref();
      },
      [&xy_now](data_staging::script& shape) {
        xy_now = shape.transitive_location_ref().position_cref();
      });

  meta_visit(
      shape_prev,
      [&xy_prev, &radius_prev](data_staging::circle& shape) {
        xy_prev = shape.transitive_location_ref().position_cref();
        radius_prev = shape.radius() + shape.radius_size();
      },
      [&xy_prev](data_staging::ellipse& shape) {
        xy_prev = shape.transitive_location_ref().position_cref();
        // ...
      },
      [&xy_prev, &xy2_prev](data_staging::line& shape) {
        xy_prev = shape.transitive_line_start_ref().position_cref();
        xy2_prev = shape.transitive_line_end_ref().position_cref();
      },
      [&xy_prev](data_staging::text& shape) {
        xy_prev = shape.transitive_location_ref().position_cref();
      },
      [&xy_prev](data_staging::script& shape) {
        xy_prev = shape.transitive_location_ref().position_cref();
      });

  auto dist = xy_now.distance_to(xy_prev);
  if (compare_xy2) dist = std::max(dist, xy2_now.distance_to(xy2_prev));

  dist += squared_dist(radius_now, radius_prev);

  return dist;
}

std::shared_ptr<data::job> generator::get_job() const {
  return job;
}

double generator::fps() const {
  return use_fps;
}
int32_t generator::width() const {
  return canvas_w;
}
int32_t generator::height() const {
  return canvas_h;
}
double generator::get_seed() const {
  return seed;
}
data::settings generator::settings() const {
  return settings_;
}
std::string generator::filename() const {
  return filename_;
}

v8::Local<v8::Value> generator::get_attr(data_staging::shape_t& spawner, v8::Local<v8::String> field) {
  auto& i = genctx->i();
  const std::string f = v8_str(i.get_isolate(), field);
  v8::Local<v8::Value> value = v8::Undefined(i.get_isolate());
  meta_callback(spawner, [&]<typename T>(const T& cc) {
    value = cc.attrs_cref().get(f);
  });
  return value;
}

int64_t generator::spawn_object(data_staging::shape_t& spawner, v8::Local<v8::Object> obj) {
  auto& i = genctx->i();

  auto instantiated_object = instantiator_.instantiate_object_from_scene(i, obj, &spawner);
  if (!instantiated_object) {
    return -1;
  }
  auto [created_instance, shape_ref_unusable, created_shape_copy] = *instantiated_object;
  create_bookkeeping_for_script_objects(created_instance, created_shape_copy);

  return i.integer_number(created_instance, "unique_id");
}

int64_t generator::spawn_object2(data_staging::shape_t& spawner, v8::Local<v8::Object> line_obj, int64_t obj1) {
  auto& i = genctx->i();
  auto uid = i.integer_number(line_obj, "unique_id");

  // create __point__ object definition
  if (object_definitions_map.find("__point__") == object_definitions_map.end()) {
    auto self_def = v8::Object::New(i.get_isolate());
    i.set_field(self_def, "x", v8::Number::New(i.get_isolate(), 0));
    i.set_field(self_def, "y", v8::Number::New(i.get_isolate(), 0));
    i.set_field(genctx->objects.Get(i.get_isolate()), "__point__", self_def);
    object_definitions_map["__point__"].Reset(i.get_isolate(), self_def);
  }

  // spawn one, we do this so we can get full transitive x and y for the line start
  // without it, the parent for example, will always be one level above the spawner,
  // typically 0,0 for example.
  auto self_obj = v8::Object::New(i.get_isolate());
  i.set_field(self_obj, "id", v8pp::to_v8(i.get_isolate(), "__point__"));
  uid = spawn_object(spawner, self_obj);

  return spawn_object3(spawner, line_obj, obj1, uid);
}

int64_t generator::spawn_object3(data_staging::shape_t& spawner,
                                 v8::Local<v8::Object> line_obj,
                                 int64_t obj1,
                                 int64_t obj2) {
  auto& i = genctx->i();
  auto instantiated_object = instantiator_.instantiate_object_from_scene(i, line_obj, &spawner);
  if (!instantiated_object) {
    return -1;
  }
  auto [created_instance, shape_ref, created_shape_copy] = *instantiated_object;
  // BEGIN: Temporary code (to try out something
  data_staging::shape_t* obj1o = nullptr;
  data_staging::shape_t* obj2o = nullptr;
  auto find1 = next_instance_map.find(obj1);
  if (find1 != next_instance_map.end()) {
    obj1o = &find1->second.get();
  } else {
    // todo; we need a mapping for this...
    for (auto& newo : scenes_.instantiated_objects_current_scene()) {
      if (auto c = std::get_if<data_staging::circle>(&newo)) {
        if (c->meta_cref().unique_id() == obj1) {
          obj1o = &newo;
        }
      }
    }
  }
  auto find2 = next_instance_map.find(obj2);
  if (find2 != next_instance_map.end()) {
    obj2o = &find2->second.get();
  } else {
    // todo; we need a mapping for this...
    for (auto& newo : scenes_.instantiated_objects_current_scene()) {
      if (auto c = std::get_if<data_staging::circle>(&newo)) {
        if (c->meta_cref().unique_id() == obj2) {
          obj2o = &newo;
        }
      }
    }
  }
  // END
  const auto handle = [](auto& line_o, auto& side_a, auto& side_b) {
    try {
      auto& line_obj = std::get<data_staging::line>(line_o);
      auto& circle_obj1 = std::get<data_staging::circle>(side_a);
      auto& circle_obj2 = std::get<data_staging::circle>(side_b);
      circle_obj1.add_cascade_out(cascade_type::start, line_obj.meta_cref().unique_id());
      circle_obj2.add_cascade_out(cascade_type::end, line_obj.meta_cref().unique_id());
      line_obj.add_cascade_in(cascade_type::start, circle_obj1.meta_cref().unique_id());
      line_obj.add_cascade_in(cascade_type::end, circle_obj2.meta_cref().unique_id());
    } catch (std::bad_variant_access const& ex) {
      logger(WARNING) << "bad variant access connecting objects" << std::endl;
      return;
    }
  };
  handle(shape_ref.get(), *obj1o, *obj2o);
  create_bookkeeping_for_script_objects(created_instance, created_shape_copy);
  return i.integer_number(created_instance, "unique_id");
}

int64_t generator::spawn_object_at_parent(data_staging::shape_t& spawner, v8::Local<v8::Object> obj) {
  auto& i = genctx->i();
  std::optional<std::reference_wrapper<data_staging::shape_t>> parent;
  meta_callback(spawner, [&]<typename T>(const T& cc) {
    if (cc.meta_cref().level() > 0) {
      parent = next_instance_map.at(cc.meta_cref().parent_uid());
    }
  });
  auto instantiated_object = ([&]() {
    if (!parent) {
      return instantiator_.instantiate_object_from_scene(i, obj, nullptr);
    }
    return instantiator_.instantiate_object_from_scene(i, obj, &((*parent).get()));
  })();
  if (!instantiated_object) {
    return -1;
  }
  auto [created_instance, shape_ref, created_shape_copy] = *instantiated_object;
  create_bookkeeping_for_script_objects(created_instance, created_shape_copy);
  return i.integer_number(created_instance, "unique_id");
}

int64_t generator::destroy(data_staging::shape_t& caller) {
  int64_t ret = -1;
  meta_callback(caller, [&]<typename T>(T& shape) {
    ret = shape.meta_cref().unique_id();
    shape.meta_ref().set_destroyed();
  });
  return ret;
}

v8::Local<v8::Object> generator::get_object(int64_t object_unique_id) {
  auto& i = genctx->i();
  // BEGIN: Temporary code (to try out something
  data_staging::shape_t* obj1o = nullptr;
  auto find1 = next_instance_map.find(object_unique_id);
  if (find1 != next_instance_map.end()) {
    obj1o = &find1->second.get();
  } else {
    // todo; we need a mapping for this...
    for (auto& newo : scenes_.instantiated_objects_current_scene()) {
      if (auto c = std::get_if<data_staging::circle>(&newo)) {
        if (c->meta_cref().unique_id() == object_unique_id) {
          obj1o = &newo;
        }
      }
    }
  }
  auto obj = v8::Object::New(i.get_isolate());

  meta_callback(*obj1o, [&]<typename T>(const T& cc) {
    i.set_field(obj, "unique_id", v8::Number::New(i.get_isolate(), cc.meta_cref().unique_id()));
    i.set_field(obj, "id", v8_str(i.get_context(), cc.meta_cref().id()));
  });
  return obj;
}

std::unordered_map<std::string, v8::Persistent<v8::Object>>& generator::get_object_definitions_ref() {
  return object_definitions_map;
}

void generator::debug_print_all() {
  logger(DEBUG) << "==[ debug print: next ]==" << std::endl;
  debug_print(scenes_.next_shapes_current_scene());

  logger(DEBUG) << "==[ debug print: intermediate ]==" << std::endl;
  debug_print(scenes_.intermediate_shapes_current_scene());

  logger(DEBUG) << "==[ debug print: current ]==" << std::endl;
  debug_print(scenes_.shapes_current_scene());
}

void generator::debug_print_next() {
  debug_print(scenes_.next_shapes_current_scene());
}

void generator::debug_print(std::vector<data_staging::shape_t>& shapes) {
  const auto print_meta = [](data_staging::meta& meta,
                             data_staging::location& loc,
                             data_staging::movement& mov,
                             data_staging::behavior& beh,
                             data_staging::generic& gen,
                             data_staging::styling& sty) {
    logger(INFO) << "uid=" << meta.unique_id() << ", puid=" << meta.parent_uid() << ", id=" << meta.id()
                 << ", level=" << meta.level() << ", namespace=" << meta.namespace_name() << " @ "
                 << loc.position_cref().x << "," << loc.position_cref().y << " +" << mov.velocity().x << ","
                 << mov.velocity().y << ", last_collide=" << beh.last_collide() << ", mass=" << gen.mass()
                 << ", angle = " << gen.angle() << ", gravity_group=" << beh.gravity_group()
                 << ", opacity=" << gen.opacity() << ", texture = " << sty.texture()
                 << ", gradient = " << sty.gradient() << ", unique_group = " << beh.unique_group()
                 << ", destroyed = " << std::boolalpha << meta.is_destroyed() << ", scale = " << gen.scale()
                 << std::endl;
  };
  for (auto& shape : shapes) {
    meta_visit(
        shape,
        [&](data_staging::circle& shape) {
          print_meta(shape.meta_ref(),
                     shape.location_ref(),
                     shape.movement_ref(),
                     shape.behavior_ref(),
                     shape.generic_ref(),
                     shape.styling_ref());
        },
        [&](data_staging::ellipse& shape) {
          print_meta(shape.meta_ref(),
                     shape.location_ref(),
                     shape.movement_ref(),
                     shape.behavior_ref(),
                     shape.generic_ref(),
                     shape.styling_ref());
        },
        [&](data_staging::line& shape) {
          print_meta(shape.meta_ref(),
                     shape.line_start_ref(),
                     shape.movement_line_start_ref(),
                     shape.behavior_ref(),
                     shape.generic_ref(),
                     shape.styling_ref());
        },
        [&](data_staging::text& shape) {
          print_meta(shape.meta_ref(),
                     shape.location_ref(),
                     shape.movement_ref(),
                     shape.behavior_ref(),
                     shape.generic_ref(),
                     shape.styling_ref());
        },
        [&](data_staging::script& shape) {
          print_meta(shape.meta_ref(),
                     shape.location_ref(),
                     shape.movement_ref(),
                     shape.behavior_ref(),
                     shape.generic_ref(),
                     shape.styling_ref());
        });
  }
}

std::vector<int64_t> generator::get_transitive_ids(const std::vector<int64_t>& unique_ids) {
  std::vector<int64_t> ret = unique_ids;
  if (mappings_dirty) {
    create_new_mappings();
  }
  try {
    auto queue = unique_ids;
    while (!queue.empty()) {
      const auto item = queue.back();
      queue.pop_back();
      const auto obj = next_instance_map.at(item).get();
      meta_callback(obj, [&]<typename T>(const T& cc) {
        if (item != cc.meta_cref().unique_id()) {
          throw std::runtime_error("next instance map lookup index corrupted");
        }
        if (cc.meta_cref().level() >= 0) {
          ret.push_back(cc.meta_cref().parent_uid());
        }
        if (cc.meta_cref().level() > 0) {
          queue.push_back(cc.meta_cref().parent_uid());
        }
      });
    }
  } catch (std::out_of_range& ex) {
    logger(WARNING) << "generator::get_transitive_ids: " << ex.what() << std::endl;
  }
  return ret;
}

}  // namespace interpreter
