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
#include <mutex>
#include <numeric>
#include <set>

#include "v8pp/module.hpp"

#include "scripting.h"
#include "starcry/metrics.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/memory_usage.hpp"
#include "util/random.hpp"
#include "util/step_calculator.hpp"
#include "util/vector_logic.hpp"

#include "data/texture.hpp"

#include "shapes/circle.h"
#include "shapes/position.h"
#include "shapes/rectangle.h"

#include "interpreter/fast_forwarder.hpp"
#include "util/unique_group.hpp"

namespace interpreter {

generator::generator(std::shared_ptr<metrics>& metrics, std::shared_ptr<v8_wrapper>& context, bool debug)
    : context(context),
      metrics_(metrics),
      debug_(debug),
      initializer_(*this),
      bridges_(*this),
      scenes_(*this),
      sampler_(*this) {}

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
  rand_.set_seed(rand_seed.value_or(0));

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

    instantiate_additional_objects_from_new_scene(genctx->scene_objects, 0);

    // since this is invoked directly after a scene change, and in the very beginning, make sure this state is part of
    // the instances "current" frame, or reverting (e.g., due to motion blur requirements) will discard all of this.
    scene_shapes = scene_shapes_next;
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
  meta_callback(created_shape, [&]<typename T>(T& shape) {
    created_shape_namespace = shape.meta_cref().namespace_name();
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
    _instantiate_object_copy_fields(i, created_instance, object_src_obj);

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
    instantiate_additional_objects_from_new_scene(tmp, debug_level + 1, &created_shape);
  }

  // clean-up temporary variable that referenced the entire imported script
  context->run(fmt::format("tmp{} = undefined;", use_index));
}

void generator::instantiate_additional_objects_from_new_scene(v8::Persistent<v8::Array>& scene_objects,
                                                              int debug_level,
                                                              const data_staging::shape_t* parent_object) {
  auto& i = genctx->i();
  std::string namespace_;

  if (parent_object)
    meta_callback(*parent_object, [&]<typename T>(const T& shape) {
      namespace_ = shape.meta_cref().namespace_name();
      logger(DEBUG) << std::string(debug_level, ' ')
                    << "Instantiating additional objects from new scene for parent object: " << shape.meta_cref().id()
                    << std::endl;
    });
  else {
    logger(DEBUG) << std::string(debug_level, ' ')
                  << "Instantiating additional objects from new scene for parent object NULL" << std::endl;
  }

  // instantiate all the additional objects from the new scene
  for (size_t j = 0; j < scene_objects.Get(i.get_isolate())->Length(); j++) {
    auto scene_obj = i.get_index(scene_objects, j).As<v8::Object>();
    auto scene_obj_id = i.str(scene_obj, "id");
    logger(DEBUG) << std::string(debug_level, ' ') << "Instantiating object id: " << scene_obj_id
                  << " (namespace: " << namespace_ << ")" << std::endl;

    auto instantiated_object = _instantiate_object_from_scene(i, scene_obj, parent_object);
    if (instantiated_object) {
      auto [created_instance, shape_ref, shape_copy] = *instantiated_object;
      create_bookkeeping_for_script_objects(created_instance, shape_copy, debug_level + 1);
    }
  }
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
          qts.clear();
          qts_gravity.clear();

          scenes_.prepare_scene();

          // create mappings (object uid -> object ref)
          create_new_mappings();

          // handle object movement (velocity added to position)
          // - velocity
          update_object_positions(i, video);

          // handle collisions, gravity and "inherited" objects
          // absolute xy are known after this function call
          // - rotations, this produces xy absolute values as side-effect
          // TODO: rotations perhaps should be part of update_positions?
          // - collisions
          // - gravity
          update_object_interactions(i, video);

          // calculate distance and steps
          update_object_distances();

          // above update functions could have triggered spawning of new objects
          insert_newly_created_objects();

          // convert javascript to renderable objects
          convert_objects_to_render_job(i, sc, video);

          scene_shapes_intermediate = scene_shapes_next;

          scalesettings.update();
          scenes_.update();

          if (job->shapes.size() != size_t(stepper.max_step)) detected_too_many_steps = true;
          metrics_->update_steps(job->job_number + 1, attempt, stepper.current_step);
        }
        if (!detected_too_many_steps) {                 // didn't bail out with break above
          if (stepper.max_step == max_intermediates) {  // config doesn't allow finer granularity any way, break.
            break;
          } else if (stepper.max_step > max_intermediates) {
            logger(INFO) << "stepper.max_step > max_intermediates -> " << stepper.max_step << " > " << max_intermediates
                         << std::endl;
            std::exit(0);
            throw std::logic_error(
                fmt::format("stepper.max_step > max_intermediates ({} > {})", stepper.max_step, max_intermediates));
          }
        }
      }

      if (!settings_.update_positions) {
        revert_position_updates(i);
      }

      cleanup_destroyed_objects();

      scene_shapes = scene_shapes_next;

      scalesettings.commit();
      scenes_.commit();
      if (debug_) {
        debug_print_next();
      }
      fpsp.inc();

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

  } catch (std::exception& ex) {
    std::cout << ex.what() << std::endl;
  }
  job->last_frame = job->frame_number == max_frames;
  return job->frame_number != max_frames;
}

void generator::revert_all_changes(v8_interact& i) {
  job->shapes.clear();
  indexes.clear();

  // reset next and intermediate instances
  scene_shapes_next = scene_shapes;
  scene_shapes_intermediate = scene_shapes;

  scalesettings.revert();
  scenes_.revert();
}

void generator::revert_position_updates(v8_interact& i) {
  // TODO
  //  for (size_t j = 0; j < next_instances->Length(); j++) {
  //    auto src = i.get_index(instances, j).As<v8::Object>();
  //    auto dst = i.get_index(next_instances, j).As<v8::Object>();
  //    auto dst2 = i.get_index(intermediates, j).As<v8::Object>();
  //    i.copy_field(dst, "x", src, "x");
  //    i.copy_field(dst, "y", src, "y");
  //    i.copy_field(dst, "x2", src, "x2");
  //    i.copy_field(dst, "y2", src, "y2");
  //    i.copy_field(dst2, "x", src, "x");
  //    i.copy_field(dst2, "y", src, "y");
  //    i.copy_field(dst2, "x2", src, "x2");
  //    i.copy_field(dst2, "y2", src, "y2");
  //  }
}

void generator::cleanup_destroyed_objects() {
  for (auto& scenes : scene_shapes_next) {
    scenes.erase(std::remove_if(scenes.begin(),
                                scenes.end(),
                                [&scenes](auto& shape) {
                                  bool ret = false;
                                  meta_callback(shape, [&]<typename T>(T& shape) {
                                    ret = shape.meta_cref().is_destroyed();
                                    if (ret) {
                                      // we should update levels for this about to be erased parent
                                      // we can do this here because this does not involve removing
                                      // elements, etc., so we won't invalidate any iterators

                                      // for now we'll just orphan all the objects that are affected
                                      // let's check only one level for now

                                      // TODO: we should do this recursively..., but this is a nice start
                                      for (auto& scene_shape : scenes) {
                                        meta_callback(scene_shape, [&]<typename T2>(T2& shape2) {
                                          if (shape2.meta_cref().parent_uid() == shape.meta_cref().unique_id()) {
                                            shape2.meta_ref().set_level(0);
                                            shape2.meta_ref().set_parent_uid(-1);
                                          }
                                        });
                                      }
                                    }
                                  });
                                  return ret;
                                }),
                 scenes.end());
  }
}

void generator::create_new_mappings() {
  next_instance_map.clear();
  intermediate_map.clear();
  for (auto& abstract_shape : scene_shapes_next[scenes_.scenesettings.current_scene_next]) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      next_instance_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
  for (auto& abstract_shape : scene_shapes_intermediate[scenes_.scenesettings.current_scene_next]) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      intermediate_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
}

void generator::update_object_positions(v8_interact& i, v8::Local<v8::Object>& video) {
  int64_t scenesettings_from_object_id = -1;
  int64_t scenesettings_from_object_id_level = -1;

  for (auto& abstract_shape : scene_shapes_next[scenes_.scenesettings.current_scene_next]) {
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      if constexpr (std::is_same_v<T, data_staging::script>) {
        // TODO: this strategy does not support nested script objects
        // TODO: we need to use stack for that
        scenesettings_from_object_id = shape.meta_cref().unique_id();
        scenesettings_from_object_id_level = shape.meta_cref().level();
      } else if (scenesettings_from_object_id_level == shape.meta_cref().level()) {
        scenesettings_from_object_id = -1;
        scenesettings_from_object_id_level = -1;
      }

      if (scenesettings_from_object_id == -1) {
        update_time(i, abstract_shape, shape.meta_cref().id(), scenes_.scenesettings);
      } else {
        // TODO:
        update_time(
            i, abstract_shape, shape.meta_cref().id(), scenes_.scenesettings_objs[scenesettings_from_object_id]);
      }

      // TODO: this was an interesting hack..
      // should not be needed. I don't think scripts should be able to customize the scale for now.
      // scalesettings.video_scale_next = i.double_number(video, "scale", 1.0);

      auto angle = shape.generic_cref().angle();
      if (std::isnan(angle)) {
        angle = 0.;
      }
      double x = 0;
      double y = 0;
      double x2 = 0;
      double y2 = 0;
      double velocity = 0;
      double vel_x = 0;
      double vel_y = 0;
      double vel_x2 = 0;
      double vel_y2 = 0;
      if constexpr (std::is_same_v<T, data_staging::line>) {
        x = shape.line_start_ref().position_cref().x;
        y = shape.line_start_ref().position_cref().y;
        x2 = shape.line_end_ref().position_cref().x;
        y2 = shape.line_end_ref().position_cref().y;
        velocity = shape.movement_line_start_ref().velocity_speed();
        vel_x = shape.movement_line_start_ref().velocity().x;
        vel_y = shape.movement_line_start_ref().velocity().y;
        vel_x2 = shape.movement_line_end_ref().velocity().x;
        vel_y2 = shape.movement_line_end_ref().velocity().y;
      } else {
        x = shape.location_cref().position_cref().x;
        y = shape.location_cref().position_cref().y;
        velocity = shape.movement_cref().velocity_speed();
        vel_x = shape.movement_cref().velocity().x;
        vel_y = shape.movement_cref().velocity().y;
      }

      velocity /= static_cast<double>(stepper.max_step);
      x += (vel_x * velocity);
      y += (vel_y * velocity);
      x2 += (vel_x2 * velocity);
      y2 += (vel_y2 * velocity);

      if constexpr (std::is_same_v<T, data_staging::line>) {
        shape.line_start_ref().position_ref().x = x;
        shape.line_start_ref().position_ref().y = y;
        shape.line_end_ref().position_ref().x = x2;
        shape.line_end_ref().position_ref().y = y2;
      } else {
        shape.location_ref().position_ref().x = x;
        shape.location_ref().position_ref().y = y;
      }
    });
  }
}

void generator::insert_newly_created_objects() {
  auto& dest = scene_shapes_next[scenes_.scenesettings.current_scene_next];
  auto& source = instantiated_objects[scenes_.scenesettings.current_scene_next];
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

void generator::update_object_toroidal(v8_interact& i, data_staging::toroidal& toroidal_data, double& x, double& y) {
  if (toroidal_data.group().empty()) return;

  auto the_width = toroidals[toroidal_data.group()].width;
  auto the_height = toroidals[toroidal_data.group()].height;
  auto diff_x = 0;
  auto diff_y = 0;

  while (x + (the_width / 2) < 0) {
    x += the_width;
    diff_x += the_width;
  }
  while (y + (the_height / 2) < 0) {
    y += the_height;
    diff_y += the_height;
  }
  while (x + (the_width / 2) > the_width) {
    x -= the_width;
    diff_x -= the_width;
  }
  while (y + (the_height / 2) > the_height) {
    y -= the_height;
    diff_y -= the_height;
  }
  const auto warped_dist = get_distance(0, 0, diff_x, diff_y);
  toroidal_data.set_warp_width(the_width);
  toroidal_data.set_warp_height(the_height);
  toroidal_data.set_warp_dist(warped_dist);
}

void generator::update_object_interactions(v8_interact& i, v8::Local<v8::Object>& video) {
  const auto handle_pass1 = [&]<typename T>(data_staging::shape_t& abstract_shape, T& shape, data_staging::meta& meta) {
    if (meta.level() >= 0) {
      if (stack.size() <= meta.level()) {
        stack.emplace_back(abstract_shape);
      } else {
        stack[meta.level()] = std::ref(abstract_shape);
      }
    }
    handle_rotations(abstract_shape, stack);

    if constexpr (std::is_same_v<T, data_staging::circle>) {
      if (shape.radius_size() < 1000 /* todo create property of course */) {
        double x = shape.transitive_location_cref().position_cref().x;
        double y = shape.transitive_location_cref().position_cref().y;
        // TODO:
        update_object_toroidal(i, shape.toroidal_ref(), x, y);
        const auto collision_group = shape.behavior_cref().collision_group();
        const auto gravity_group = shape.behavior_cref().gravity_group();
        const auto unique_group = shape.behavior_cref().unique_group();

        if (!collision_group.empty()) {
          qts.try_emplace(collision_group,
                          quadtree(rectangle(position(-width() / 2, -height() / 2), width(), height()), 32));
          qts[collision_group].insert(point_type(position(x, y), shape.meta_cref().unique_id()));
        }
        if (!gravity_group.empty()) {
          qts_gravity.try_emplace(gravity_group,
                                  quadtree(rectangle(position(-width() / 2, -height() / 2), width(), height()), 32));
          qts_gravity[gravity_group].insert(point_type(position(x, y), shape.meta_cref().unique_id()));
        }
        if (!unique_group.empty()) {
          unique_groups[unique_group].add(x, y);
        }
      }
    }
    if constexpr (std::is_same_v<T, data_staging::line>) {
      double x = shape.transitive_line_start_ref().position_ref().x;
      double y = shape.transitive_line_start_ref().position_ref().y;
      double x2 = shape.transitive_line_end_ref().position_ref().x;
      double y2 = shape.transitive_line_end_ref().position_ref().y;
      const auto unique_group = shape.behavior_cref().unique_group();
      if (!unique_group.empty()) {
        unique_groups[unique_group].add(x, y, x2, y2);
      }
    }
  };

  const auto handle_pass2 = [&]<typename T>(data_staging::shape_t& abstract_shape, T& shape, data_staging::meta& meta) {
    handle_collisions(i, abstract_shape);
    handle_gravity(i, abstract_shape);
  };

  // first pass transitive xy become set
  stack.clear();
  for (auto& abstract_shape : scene_shapes_next[scenes_.scenesettings.current_scene_next]) {
    std::visit(overloaded{[](std::monostate) {},
                          [&](data_staging::circle& c) {
                            handle_pass1(abstract_shape, c, c.meta_ref());
                          },
                          [&](data_staging::line& l) {
                            handle_pass1(abstract_shape, l, l.meta_ref());
                          },
                          [&](data_staging::text& t) {
                            handle_pass1(abstract_shape, t, t.meta_ref());
                          },
                          [&](data_staging::script& s) {
                            handle_pass1(abstract_shape, s, s.meta_ref());
                          }},
               abstract_shape);
  }

  // second pass depends on knowing transitive xy
  for (auto& abstract_shape : scene_shapes_next[scenes_.scenesettings.current_scene_next]) {
    std::visit(overloaded{[](std::monostate) {},
                          [&](data_staging::circle& c) {
                            handle_pass2(abstract_shape, c, c.meta_ref());
                            for (const auto& cascade_out : c.cascade_out_cref()) {
                              auto& other = next_instance_map.at(cascade_out.unique_id()).get();
                              if (auto other_line = std::get_if<data_staging::line>(&other)) {
                                if (cascade_out.type() == cascade_type::start) {
                                  other_line->line_start_ref().position_ref().x = c.location_ref().position_cref().x;
                                  other_line->line_start_ref().position_ref().y = c.location_ref().position_cref().y;
                                  other_line->transitive_line_start_ref().position_ref().x =
                                      c.transitive_location_ref().position_cref().x;
                                  other_line->transitive_line_start_ref().position_ref().y =
                                      c.transitive_location_ref().position_cref().y;
                                } else if (cascade_out.type() == cascade_type::end) {
                                  other_line->line_end_ref().position_ref().x = c.location_ref().position_cref().x;
                                  other_line->line_end_ref().position_ref().y = c.location_ref().position_cref().y;
                                  other_line->transitive_line_end_ref().position_ref().x =
                                      c.transitive_location_ref().position_cref().x;
                                  other_line->transitive_line_end_ref().position_ref().y =
                                      c.transitive_location_ref().position_cref().y;
                                }
                              }
                            }
                          },
                          [&](data_staging::line& l) {
                            handle_pass2(abstract_shape, l, l.meta_ref());
                          },
                          [&](data_staging::text& t) {
                            handle_pass2(abstract_shape, t, t.meta_ref());
                          },
                          [&](data_staging::script& s) {
                            handle_pass2(abstract_shape, s, s.meta_ref());
                          }},
               abstract_shape);
  }
}

void generator::update_object_distances() {
  stepper.reset_current();
  const auto handle = [&](data_staging::shape_t& abstract_shape, data_staging::meta& meta) {
    auto instance_uid = meta.unique_id();
    // MARK
    auto find = intermediate_map.find(instance_uid);
    if (find == intermediate_map.end()) {
      return;
    }
    double dist = get_max_travel_of_object(abstract_shape, find->second.get());
    if (dist > max_dist_found) {
      max_dist_found = dist;
    }
    auto steps = update_steps(dist);

    static std::unordered_map<int64_t, int> recorded_steps;

    if (attempt == 1) {
      meta.set_distance(dist);
      meta.set_steps(steps);
      recorded_steps[instance_uid] = steps;
    } else if (attempt > 1) {
      meta.set_steps(recorded_steps[instance_uid]);
    }
  };
  for (auto& abstract_shape : scene_shapes_next[scenes_.scenesettings.current_scene_next]) {
    meta_callback(abstract_shape, [&]<typename TP>(TP& shape) {
      handle(abstract_shape, shape.meta_ref());
    });
  }
}

void generator::handle_rotations(data_staging::shape_t& shape,
                                 std::vector<std::reference_wrapper<data_staging::shape_t>>& use_stack) {
  vector2d parent;
  vector2d new_position;
  vector2d new_position2;
  bool pivot_found = false;

  const auto handle = [&]<typename T>(T concrete_shape) {
    // we'll iterate over all the parents of the current shape, and the shape itself, starting at the top pos_parent.
    // in case the current shape is at level 3, we iterate over 0, 1, 2, 3.
    vector2d current;
    vector2d current1;
    vector2d current2;
    double current_rotation = 0;
    for (size_t i = 0; i <= concrete_shape.meta_cref().level(); i++) {
      meta_callback(use_stack[i].get(), [&]<typename TP>(TP& parent_shape) {
        if constexpr (std::is_same_v<TP, data_staging::circle>) {
          current = add_vector(current, parent_shape.location_ref().position_ref());
        } else if constexpr (std::is_same_v<TP, data_staging::text>) {
          current = add_vector(current, parent_shape.location_ref().position_ref());
        } else if constexpr (std::is_same_v<TP, data_staging::line>) {
          current1 = add_vector(current, parent_shape.line_start_ref().position_ref());
          current2 = add_vector(current, parent_shape.line_end_ref().position_ref());
          current = add_vector(
              current, {((current1.x - current2.x) / 2) + current2.x, ((current1.y - current2.y) / 2) + current2.y});
        } else if constexpr (std::is_same_v<TP, data_staging::script>) {
          current = add_vector(current, parent_shape.location_ref().position_ref());
        }

        double angle = parent_shape.generic_ref().angle();
        current_rotation += parent_shape.generic_ref().rotate();

        const auto rotate = [&](auto& current, auto& parent) {
          auto angle1 = current_rotation + angle + get_angle(parent.x, parent.y, current.x, current.y);
          angle1 = std::fmod(angle1, 360.);
          auto rads = angle1 * M_PI / 180.0;
          auto ratio = 1.0;
          auto dist = get_distance(parent.x, parent.y, current.x, current.y);
          auto move = dist * ratio * -1;
          // now current will be adjusted with the rotation
          current.x = parent.x + (cos(rads) * move);
          current.y = parent.y + (sin(rads) * move);
        };

        if constexpr (std::is_same_v<TP, data_staging::line>) {
          rotate(current1, parent);
          rotate(current2, parent);
          new_position = current1;
          new_position2 = current2;
        } else {
          rotate(current, parent);
          new_position = current;
        }

        parent = current;

        if (!pivot_found && parent_shape.meta_cref().is_pivot()) {
          // skip to the last element in the stack, i - 1 due to i++ every iteration
          i = concrete_shape.meta_cref().level() - 1;
          pivot_found = true;
        }
      });
    }
  };

  meta_visit(
      shape,
      [&handle, &new_position](data_staging::circle& c) {
        handle(c);
        c.transitive_location_ref().position_ref().x = new_position.x;
        c.transitive_location_ref().position_ref().y = new_position.y;
      },
      [&handle, &new_position, &new_position2](data_staging::line& l) {
        handle(l);
        bool skip_start = false, skip_end = false;
        for (const auto& cascade_in : l.cascades_in()) {
          if (cascade_in.type() == cascade_type::start) {
            skip_start = true;
          }
          if (cascade_in.type() == cascade_type::end) {
            skip_end = true;
          }
        }
        if (!skip_start) {
          l.transitive_line_start_ref().position_ref().x = new_position.x;
          l.transitive_line_start_ref().position_ref().y = new_position.y;
        }
        if (!skip_end) {
          l.transitive_line_end_ref().position_ref().x = new_position2.x;
          l.transitive_line_end_ref().position_ref().y = new_position2.y;
        }
      },
      [&handle, &new_position](data_staging::text& t) {
        handle(t);
        t.transitive_location_ref().position_ref().x = new_position.x;
        t.transitive_location_ref().position_ref().y = new_position.y;
      },
      [&handle, &new_position](data_staging::script& s) {
        handle(s);
        s.transitive_location_ref().position_ref().x = new_position.x;
        s.transitive_location_ref().position_ref().y = new_position.y;
      });
}

void generator::handle_collisions(v8_interact& i, data_staging::shape_t& shape) {
  // Now do the collision detection part
  // NOTE: we multiple radius/size * 2.0 since we're not looking up a point, and querying the quadtree does
  // not check for overlap, but only whether the x,y is inside the specified range. If we don't want to miss
  // points on the edge of our circle, we need to widen the matching range.
  // TODO: for different sizes of circle collision detection, we need to somehow adjust the interface to this
  // lookup somehow.
  std::vector<point_type> found;
  try {
    data_staging::circle& c = std::get<data_staging::circle>(shape);
    const auto& collision_group = c.behavior_ref().collision_group_ref();
    if (collision_group.empty() || collision_group == "undefined") {
      return;
    }
    auto x = c.transitive_location_ref().position_ref().x;
    auto y = c.transitive_location_ref().position_ref().y;
    auto unique_id = c.meta_cref().unique_id();

    if (c.meta_cref().id() == "balls") return;

    auto radius = c.radius();
    auto radiussize = c.radius_size();

    qts[collision_group].query(unique_id, circle(position(x, y), radius * 2.0, radiussize * 2.0), found);
    if (radiussize < 1000 /* todo create property of course */) {
      for (const auto& collide : found) {
        const auto unique_id2 = collide.userdata;
        auto& shape2 = next_instance_map.at(unique_id2);
        try {
          data_staging::circle& c2 = std::get<data_staging::circle>(shape2.get());
          if (c2.meta_cref().id() != "balls" && c.meta_cref().unique_id() != c2.meta_cref().unique_id()) {
            handle_collision(i, c, c2, shape, shape2.get());
          }
        } catch (std::bad_variant_access const& ex) {
          // only supporting circles for now
          return;
        }
      }
    }
  } catch (std::bad_variant_access const& ex) {
    // only supporting circles for now
    return;
  }
}

void generator::handle_collision(v8_interact& i,
                                 data_staging::circle& instance,
                                 data_staging::circle& instance2,
                                 data_staging::shape_t& shape,
                                 data_staging::shape_t& shape2) {
  auto unique_id = instance.meta_cref().unique_id();
  auto unique_id2 = instance2.meta_cref().unique_id();
  auto last_collide = instance.behavior_ref().last_collide();

  auto x = instance.transitive_location_ref().position_cref().x;
  auto y = instance.transitive_location_ref().position_cref().y;

  auto x2 = instance2.transitive_location_ref().position_cref().x;
  auto y2 = instance2.transitive_location_ref().position_cref().y;

  auto radius = instance.radius();
  auto radiussize = instance.radius_size();
  auto radius2 = instance2.radius();
  auto radiussize2 = instance2.radius_size();
  auto mass = instance.generic_cref().mass();
  auto mass2 = instance2.generic_cref().mass();

  // If the quadtree reported a match, it doesn't mean the objects fully collide
  circle a(position(x, y), radius, radiussize);
  circle b(position(x2, y2), radius2, radiussize2);
  if (!a.overlaps(b)) return;

  // they already collided, no need to let them collide again
  if (last_collide == unique_id2) return;

  // handle collision
  auto vel_x = instance.movement_ref().velocity().x;
  auto vel_y = instance.movement_ref().velocity().y;
  auto vel_x2 = instance2.movement_ref().velocity().x;
  auto vel_y2 = instance2.movement_ref().velocity().y;

  const auto normal = unit_vector(subtract_vector(vector2d(x, y), vector2d(x2, y2)));
  const auto ta = dot_product(vector2d(vel_x, vel_y), normal);
  const auto tb = dot_product(vector2d(vel_x2, vel_y2), normal);
  const auto optimized_p = (2.0 * (ta - tb)) / (mass + mass2);  // speed

  // save velocities
  const auto multiplied_vector = multiply_vector(normal, optimized_p);
  auto updated_vel1 = subtract_vector(vector2d(vel_x, vel_y), multiply_vector(multiplied_vector, mass2));
  instance.movement_ref().set_velocity(updated_vel1);
  auto updated_vel2 = add_vector(vector2d(vel_x2, vel_y2), multiply_vector(multiplied_vector, mass));
  instance2.movement_ref().set_velocity(updated_vel2);

  // save collision
  instance.behavior_ref().set_last_collide(unique_id2);
  instance2.behavior_ref().set_last_collide(unique_id);

  // collide callback
  auto find = object_definitions_map.find(instance.meta_cref().id());
  if (find != object_definitions_map.end()) {
    auto object_definition = v8::Local<v8::Object>::New(i.get_isolate(), find->second);
    auto handle_time_for_shape = [&](auto& c, auto& object_bridge, auto other_unique_id) {
      object_bridge->push_object(c);
      i.call_fun(object_definition, object_bridge->instance(), "collide", other_unique_id);
      object_bridge->pop_object();
    };
    auto callback_wrapper = [&]<typename T>(T& shape, int64_t unique_id) {
      if constexpr (std::is_same_v<T, data_staging::circle>) {
        return handle_time_for_shape(shape, bridges_.circle(), unique_id);
      } else if constexpr (std::is_same_v<T, data_staging::line>) {
        return handle_time_for_shape(shape, bridges_.line(), unique_id);
      } else if constexpr (std::is_same_v<T, data_staging::text>) {
        return handle_time_for_shape(shape, bridges_.text(), unique_id);
      } else if constexpr (std::is_same_v<T, data_staging::script>) {
        return handle_time_for_shape(shape, bridges_.script(), unique_id);
      }
      // unknown (undefined) objects, are ignored..
    };
    meta_callback(shape, [&]<typename T>(T& shape) {
      callback_wrapper(shape, unique_id2);
    });
    meta_callback(shape2, [&]<typename T>(const T& shape) {
      callback_wrapper(shape, unique_id);
    });
  }
}

void generator::handle_gravity(v8_interact& i, data_staging::shape_t& shape) {
  try {
    std::vector<point_type> found;
    data_staging::circle& c = std::get<data_staging::circle>(shape);

    auto unique_id = c.meta_cref().unique_id();
    auto gravity_group = c.behavior_ref().gravity_group();
    if (gravity_group.empty()) {
      return;
    }

    if (c.movement_ref().velocity_speed() == 0) return;  // skip this one.

    auto x = c.transitive_location_ref().position_cref().x;
    auto y = c.transitive_location_ref().position_cref().y;

    auto radius = c.radius();
    auto radiussize = c.radius_size();

    auto& video = genctx->video_obj;
    auto G = i.double_number(video, "gravity_G", 1);
    auto range = i.double_number(video, "gravity_range", 1000);
    const auto constrain_dist_min = i.double_number(video, "gravity_constrain_dist_min", 5.);
    const auto constrain_dist_max = i.double_number(video, "gravity_constrain_dist_max", 25.);

    qts_gravity[gravity_group].query(
        unique_id, circle(position(x, y), range + (radius * 2.0), range + (radiussize * 2.0)), found);

    if (c.radius_size() < 1000 /* todo create property of course */) {
      vector2d acceleration(0, 0);
      for (const auto& in_range : found) {
        const auto unique_id2 = in_range.userdata;
        auto shape2 = next_instance_map.at(unique_id2);
        try {
          data_staging::circle& c2 = std::get<data_staging::circle>(shape2.get());
          if (c.meta_cref().unique_id() != c2.meta_cref().unique_id()) {
            handle_gravity(c, c2, acceleration, G, range, constrain_dist_min, constrain_dist_max);
          }
        } catch (std::bad_variant_access const& ex) {
          // only supporting circles for now
          return;
        }
      }
      auto vel = add_vector(c.movement_ref().velocity(), acceleration);
      c.movement_ref().set_velocity(vel);
    }
  } catch (std::bad_variant_access const& ex) {
    // only supporting circles for now
    return;
  }
  //---
}

void generator::handle_gravity(data_staging::circle& instance,
                               data_staging::circle& instance2,
                               vector2d& acceleration,
                               double G,
                               double range,
                               double constrain_dist_min,
                               double constrain_dist_max) const {
  // auto unique_id = instance.meta_cref().unique_id();
  auto x = instance.transitive_location_ref().position_cref().x;
  auto y = instance.transitive_location_ref().position_cref().y;

  // auto unique_id2 = instance.meta_cref().unique_id();
  auto x2 = instance2.transitive_location_ref().position_cref().x;
  auto y2 = instance2.transitive_location_ref().position_cref().y;

  auto radius = instance.radius();
  auto radiussize = instance.radius_size();
  auto radius2 = instance2.radius();
  auto radiussize2 = instance2.radius_size();
  auto mass = instance.generic_ref().mass();
  auto mass2 = instance2.generic_ref().mass();

  // If the quadtree reported a match, it doesn't mean the objects fully collide
  circle a(position(x, y), radius + range, radiussize);
  circle b(position(x2, y2), radius2 + range, radiussize2);
  double dist = 0;
  if (!a.overlaps(b, dist)) return;

  const auto constrained_distance = std::clamp(dist, constrain_dist_min, constrain_dist_max);

  vector2d vec_a(x, y);
  vector2d vec_b(x2, y2);
  const auto strength = (G * mass * mass2) / (constrained_distance * constrained_distance);
  auto force = subtract_vector(vec_b, vec_a);
  force = unit_vector(force);
  force = multiply_vector(force, strength / static_cast<double>(stepper.max_step));
  force = divide_vector(force, mass);

  acceleration.x += force.x;
  acceleration.y += force.y;
}

std::shared_ptr<v8_wrapper> generator::get_context() const {
  return context;
}

void generator::update_time(v8_interact& i,
                            data_staging::shape_t& instance,
                            const std::string& instance_id,
                            scene_settings& scenesettings) {
  const auto time_settings = scenes_.get_time(scenesettings);
  const auto execute = [&](double scene_time) {
    // Are these still needed?? (EDIT: I don't think it's used)
    // EDIT: during texture rendering we query the shape for the time
    // i.set_field(instance, "__time__", v8::Number::New(i.get_isolate(), scene_time));
    // i.set_field(instance, "__global_time__", v8::Number::New(i.get_isolate(), time_settings.time));
    // i.set_field(instance, "__elapsed__", v8::Number::New(i.get_isolate(), time_settings.elapsed));

    auto find = object_definitions_map.find(instance_id);
    if (find != object_definitions_map.end()) {
      auto object_definition = v8::Local<v8::Object>::New(i.get_isolate(), find->second);
      auto handle_time_for_shape = [&](auto& c, auto& object_bridge) {
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
    auto bak = scenesettings.current_scene_next;
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

void generator::convert_objects_to_render_job(v8_interact& i, step_calculator& sc, v8::Local<v8::Object> video) {
  //  // Risking doing this for nothing, as this may still be discarded, we'll translate all the instances to
  //  // objects ready for rendering Note that we save object states for multiple "steps" per frame if needed.
  //  for (size_t index = 0; index < next_instances->Length(); index++) {
  //    auto instance = i.get_index(next_instances, index).As<v8::Object>();
  //    if (!instance->IsObject()) continue;
  //    convert_object_to_render_job(i, instance, index, sc, video);
  //  }
  for (auto& shape : scene_shapes_next[scenes_.scenesettings.current_scene_next]) {
    bool skip = false;
    meta_callback(shape, [&]<typename T>(const T& shape) {
      if (shape.meta_cref().is_destroyed()) {
        skip = true;
      }
    });
    if (!skip) convert_object_to_render_job(i, shape, sc, video);
  }
}

void generator::convert_object_to_render_job(v8_interact& i,
                                             data_staging::shape_t& shape,
                                             step_calculator& sc,
                                             v8::Local<v8::Object> video) {
  data::shape new_shape;

  const auto initialize = [&]<typename T>(T& shape) {
    auto level = 0;  // shape.level;
    // See if we require this step for this object
    // auto steps = i.integer_number(instance, "steps");
    // if (minimize_steps_per_object && !sc.do_step(steps, stepper.next_step)) {
    // TODO: make this a property also for objects, if they are vibrating they need this
    //  return;
    //}
    // auto id = i.str(instance, "id");
    // auto label = i.str(instance, "label");
    // auto time = i.double_number(instance, "__time__");

    // auto radius = shape.radius();           // i.double_number(instance, "radius");
    // auto radiussize = shape.radius_size();  // i.double_number(instance, "radiussize");
    auto seed = this->seed;
    if constexpr (std::is_same_v<T, data_staging::circle> || std::is_same_v<T, data_staging::line>) {
      seed = shape.styling_cref().seed();
    }
    auto scale = shape.generic_cref().scale();

    auto shape_opacity = shape.generic_cref().opacity();
    auto warp_width = shape.toroidal_ref().warp_width();
    auto warp_height = shape.toroidal_ref().warp_height();

    // auto text_font = i.has_field(instance, "text_font") ? i.str(instance, "text_font") : "";

    // TODO: might not need this param after all
// auto dist = i.double_number(instance, "__dist__");
#define DEBUG_NUM_SHAPES
#ifdef DEBUG_NUM_SHAPES
// auto random_hash = i.str(instance, "__random_hash__");
#endif

    // temp
    new_shape.level = level;
    new_shape.time = shape.meta_cref().get_time();
    // new_shape.dist = dist;

    new_shape.gradients_.clear();
    new_shape.textures.clear();
    std::string gradient_id_str;

    copy_gradient_from_object_to_shape(shape, new_shape, gradients);
    copy_texture_from_object_to_shape(shape, new_shape, textures);

    // temp hack
    std::string namespace_;
    std::string gradient_id;
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      gradient_id = shape.styling_cref().gradient();
      new_shape.hue = shape.styling_cref().hue();
    }

    if (!gradient_id.empty()) {
      if (new_shape.gradients_.empty()) {
        auto& known_gradients_map = gradients;
        if (known_gradients_map.contains(gradient_id)) {
          new_shape.gradients_.emplace_back(1.0, known_gradients_map[gradient_id]);
        }
      }
    }

    if (new_shape.gradients_.empty()) {
      new_shape.gradients_.emplace_back(1.0, data::gradient{});
      new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
      new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
      new_shape.gradients_[0].second.colors.emplace_back(1.0, data::color{0.0, 0, 0, 1});
    }
    new_shape.z = 0;
    // new_shape.vel_x = vel_x;
    // new_shape.vel_y = vel_y;
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      new_shape.blending_ = shape.styling_cref().blending_type();
    }
    new_shape.scale = scale;
    new_shape.opacity = std::isnan(shape_opacity) ? 1.0 : shape_opacity;
    // new_shape.unique_id = unique_id;
#ifdef DEBUG_NUM_SHAPES
    // new_shape.random_hash = random_hash;
#endif
    new_shape.seed = seed;
    new_shape.id = shape.meta_cref().id();
    // new_shape.label = label;
    // new_shape.motion_blur = motion_blur;
    new_shape.warp_width = warp_width;
    new_shape.warp_height = warp_height;

    // }
    // wrap this in a proper add method
    if (stepper.next_step != stepper.max_step) {
      indexes[shape.meta_cref().unique_id()][stepper.current_step] = job->shapes[stepper.current_step].size();
    } else {
      new_shape.indexes = indexes[shape.meta_cref().unique_id()];
    }
    // logger(INFO) << "sizeof shape: " << sizeof(new_shape) << " circle was size: " << sizeof(shape) <<
    // std::endl;
    //                   if (job->shapes[stepper.current_step].capacity() < 10000) {
    //                     logger(INFO) << "resizing to fix capacity" << std::endl;
    //                     job->shapes[stepper.current_step].reserve(10000);
    //                   }
    // why is this shit super slow
    job->shapes[stepper.current_step].emplace_back(std::move(new_shape));
    // and this reasonably fast
    // job->shapes_prototype_test[stepper.current_step].emplace_back(shape);
    // job->shapes[stepper.current_step].emplace_back(data::shape{});
    //                   if (job->shapes[stepper.current_step].size() > 9999)
    //                     logger(INFO) << "current_step = " << stepper.current_step << ", shapes: " <<
    //                     job->shapes[stepper.current_step].size() << std::endl;
    job->scale = scalesettings.video_scale;
    job->scales = scalesettings.video_scales;
  };

  // Update level for all objects
  meta_visit(
      shape,
      [&](data_staging::circle& shape) {
        new_shape.type = data::shape_type::circle;
        new_shape.radius = shape.radius();
        new_shape.radius_size = shape.radius_size();
        new_shape.x = shape.transitive_location_ref().position_cref().x;
        new_shape.y = shape.transitive_location_ref().position_cref().y;
        initialize(shape);
      },
      [&](data_staging::line& shape) {
        new_shape.type = data::shape_type::line;
        new_shape.radius = 0;
        new_shape.radius_size = shape.line_width();
        new_shape.x = shape.transitive_line_start_ref().position_cref().x;
        new_shape.y = shape.transitive_line_start_ref().position_cref().y;
        new_shape.x2 = shape.transitive_line_end_ref().position_cref().x;
        new_shape.y2 = shape.transitive_line_end_ref().position_cref().y;
        initialize(shape);
      },
      [&](data_staging::text& shape) {
        new_shape.type = data::shape_type::text;
        new_shape.text_font = shape.font_name();
        new_shape.text = shape.text_cref();
        new_shape.text_size = shape.text_size();
        new_shape.align = shape.text_align();
        new_shape.text_fixed = shape.text_fixed();
        // TODO: new_shape.text_font = shape.text_font();
        new_shape.x = shape.transitive_location_ref().position_cref().x;
        new_shape.y = shape.transitive_location_ref().position_cref().y;
        initialize(shape);
      },
      [&](data_staging::script& shape) {
        new_shape.type = data::shape_type::script;
        new_shape.x = shape.transitive_location_ref().position_cref().x;
        new_shape.y = shape.transitive_location_ref().position_cref().y;
        initialize(shape);
      });
}

std::shared_ptr<data::job> generator::get_job() const {
  return job;
}

int64_t generator::spawn_object(data_staging::shape_t& spawner, v8::Local<v8::Object> obj) {
  auto& i = genctx->i();

  auto instantiated_object = _instantiate_object_from_scene(i, obj, &spawner);
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
  auto instantiated_object = _instantiate_object_from_scene(i, line_obj, &spawner);
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
    for (auto& newo : instantiated_objects[scenes_.scenesettings.current_scene_next]) {
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
    for (auto& newo : instantiated_objects[scenes_.scenesettings.current_scene_next]) {
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
  if (!parent) {
    return -1;
  }
  auto instantiated_object = _instantiate_object_from_scene(i, obj, &((*parent).get()));
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

std::unordered_map<std::string, v8::Persistent<v8::Object>>& generator::get_object_definitions_ref() {
  return object_definitions_map;
}

void recursively_build_stack_for_object(auto& new_stack,
                                        auto& shape,
                                        auto& next_instance_map,
                                        std::vector<data_staging::shape_t>& instantiated_objs,
                                        int level = 0) {
  meta_callback(shape, [&](const auto& cc) {
    new_stack.emplace_back(shape);
    if (cc.meta_cref().level() > 0) {
      if (next_instance_map.find(cc.meta_cref().parent_uid()) != next_instance_map.end()) {
        recursively_build_stack_for_object(new_stack,
                                           next_instance_map.at(cc.meta_cref().parent_uid()).get(),
                                           next_instance_map,
                                           instantiated_objs,
                                           level + 1);
      } else {
        for (auto& v : instantiated_objs) {
          meta_callback(v, [&](const auto& ccc) {
            if (ccc.meta_cref().unique_id() == cc.meta_cref().parent_uid()) {
              recursively_build_stack_for_object(new_stack, v, next_instance_map, instantiated_objs, level + 1);
            }
          });
        }
      }
    }
  });
}

std::optional<std::tuple<v8::Local<v8::Object>, std::reference_wrapper<data_staging::shape_t>, data_staging::shape_t>>
generator::_instantiate_object_from_scene(
    v8_interact& i,
    v8::Local<v8::Object>& scene_object,           // object description from scene to be instantiated
    const data_staging::shape_t* parent_object) {  // it's optional parent
  v8::Isolate* isolate = i.get_isolate();

  int64_t current_level = 0;
  auto parent_object_ns = std::string("");
  int64_t parent_uid = -1;

  if (parent_object) {
    meta_callback(*parent_object, [&]<typename T>(const T& cc) {
      current_level = cc.meta_cref().level() + 1;
      parent_object_ns = cc.meta_cref().namespace_name();
      parent_uid = cc.meta_cref().unique_id();
    });
  }

  // lookup the object prototype to be instantiated
  auto object_id = i.str(scene_object, "id", "");
  if (object_id != "__point__") {
    object_id = parent_object_ns + object_id;
  }
  auto object_prototype = v8_index_object(i.get_context(), genctx->objects, object_id).template As<v8::Object>();

  // logger(DEBUG) << "instantiate_object_from_scene, prototype: " << object_id << std::endl;

  // create a new javascript object
  v8::Local<v8::Object> instance = v8::Object::New(isolate);

  // TODO: make sure this is the only source..., and get rid of genctx->objects usage
  if (!object_prototype->IsObject()) {
    object_prototype = object_definitions_map[object_id].Get(isolate);
  }

  if (!object_prototype->IsObject()) {
    logger(WARNING) << "cannot instantiate object id: " << object_id << ", does not exist" << std::endl;
    throw std::runtime_error(fmt::format("cannot instantiate object id: {}, does not exist", object_id));
  }

  // instantiate the prototype into newly allocated javascript object
  _instantiate_object(i, scene_object, object_prototype, instance, current_level, parent_object_ns);

  // inherit some fields from parent
  if (parent_object) {
    meta_visit(
        const_cast<data_staging::shape_t&>(*parent_object),
        [&](data_staging::circle& shape) {
          i.set_field(instance, "gradient", v8_str(i.get_context(), shape.styling_ref().gradient()));
          // this is needed for example, if a scene object defines it, it will be inherited, but only for the first
          // instantiated object. further spawned child objects will not inherit this by default we may need to do
          // something like this for more cases..
          i.set_field(instance, "unique_group", v8_str(i.get_context(), shape.behavior_ref().unique_group()));
        },
        [&](data_staging::line& shape) {
          i.set_field(instance, "gradient", v8_str(i.get_context(), shape.styling_ref().gradient()));
          i.set_field(instance, "unique_group", v8_str(i.get_context(), shape.behavior_ref().unique_group()));
        },
        [&](data_staging::text& shape) {
          i.set_field(instance, "gradient", v8_str(i.get_context(), shape.styling_ref().gradient()));
          i.set_field(instance, "unique_group", v8_str(i.get_context(), shape.behavior_ref().unique_group()));
        },
        [&](data_staging::script& shape) {
          i.set_field(instance, "unique_group", v8_str(i.get_context(), shape.behavior_ref().unique_group()));
        });
  }

  // give it a unique id (it already has been assigned a __random_hash__ for debugging purposes
  static int64_t counter = 0;
  i.set_field(instance, "unique_id", v8::Number::New(i.get_isolate(), ++counter));
  i.set_field(instance, "parent_uid", v8::Number::New(i.get_isolate(), parent_uid));

  // TODO: in the future we will simply instantiate this directly, for now, to save some refactoring time
  // we will map, to see if the proof of concept works

  // TODO: try to use a raw pointer, see if it improves performance
  std::optional<std::reference_wrapper<data_staging::shape_t>> shape_ref;
  std::optional<data_staging::shape_t> shape_copy;

  const auto handle = [&]<typename T>(T& c) -> data_staging::shape_t& {
    // we buffer instantiated objects, and will insert in the array later.
    instantiated_objects[scenes_.scenesettings.current_scene_next].emplace_back(c);
    return instantiated_objects[scenes_.scenesettings.current_scene_next].back();
  };

  const auto type = i.str(object_definitions_map[object_id], "type", "");
  bool check_uniqueness = false;
  std::string unique_group = "";

  const auto initialize = [&]<typename T>(T& c, auto& bridge) {
    c.meta_ref().set_level(current_level);
    c.meta_ref().set_parent_uid(parent_uid);
    c.meta_ref().set_pivot(i.boolean(instance, "pivot"));
    c.behavior_ref().set_collision_group(i.str(instance, "collision_group", ""));
    c.behavior_ref().set_gravity_group(i.str(instance, "gravity_group", ""));
    c.behavior_ref().set_unique_group(i.str(instance, "unique_group", ""));
    check_uniqueness = c.behavior_ref().unique_group_ref().size();
    unique_group = c.behavior_ref().unique_group_ref();
    c.toroidal_ref().set_group(i.str(instance, "toroidal", ""));
    c.generic_ref().set_angle(i.double_number(instance, "angle", 0));
    c.generic_ref().set_rotate(i.double_number(instance, "rotate", 0));
    c.styling_ref().set_hue(i.double_number(instance, "hue", 0));
    c.generic_ref().set_opacity(i.double_number(instance, "opacity", 1));
    c.generic_ref().set_mass(i.double_number(instance, "mass", 1));
    c.generic_ref().set_scale(i.double_number(instance, "scale", 1));
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      if (i.has_field(instance, "texture")) {
        c.styling_ref().set_texture(i.str(instance, "texture"));
      }
      if (i.has_field(instance, "gradient")) {
        c.styling_ref().set_gradient(i.str(instance, "gradient"));
      }
    }
    if (i.has_field(instance, "gradients")) {
      auto gradient_array = i.v8_array(instance, "gradients");
      for (size_t k = 0; k < gradient_array->Length(); k++) {
        auto gradient_data = i.get_index(gradient_array, k).As<v8::Array>();
        if (!gradient_data->IsArray()) continue;
        auto opacity = i.double_number(gradient_data, size_t(0));
        auto gradient_id = parent_object_ns + i.str(gradient_data, size_t(1));
        if constexpr (!std::is_same_v<T, data_staging::script>) {
          c.styling_ref().add_gradient(opacity, gradient_id);
        }
      }
    }
    if (i.has_field(instance, "props")) {
      auto props_object = i.v8_obj(instance, "props");
      auto obj_fields = i.prop_names(props_object);
      for (size_t k = 0; k < obj_fields->Length(); k++) {
        auto field_name = i.get_index(obj_fields, k);
        auto field_name_str = i.str(obj_fields, k);
        auto field_value = i.get(props_object, field_name);
        i.set_field(c.properties_ref().properties_ref(), field_name, field_value);
      }
    }

    // the handle function returns a ref, which is all fine, but recursively init
    // may actually invalidate this ref with other inserts.
    shape_ref = std::ref(handle(c));
    shape_copy = (*shape_ref).get();

    // call init last, so that objects exist when using 'this' inside init()
    if (bridge) {
      // take a copy as the reference might point to a non-existant instance at some point,
      // for example when other cascading 'init's insert new objects.
      auto copy = std::get<T>((*shape_ref).get());
      bridge->push_object(copy);

      i.call_fun(object_definitions_map[object_id],  // object definition
                 bridge->instance(),                 // bridged object is "this"
                 "init");
      bridge->pop_object();
      write_back_copy(copy);
    }
  };

  if (type == "circle" || type.empty() /* treat every "non-type" as circles too */) {
    data_staging::circle c(object_id,
                           counter,
                           vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                           i.double_number(instance, "radius"),
                           i.double_number(instance, "radiussize"));

    if (type.empty()) c.generic_ref().set_opacity(0);

    c.movement_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                  i.double_number(instance, "vel_y", 0),
                                  i.double_number(instance, "velocity", 0));

    c.meta_ref().set_namespace(parent_object_ns);

    c.styling_ref().set_seed(i.integer_number(instance, "seed", 0));
    c.styling_ref().set_blending_type(i.integer_number(instance, "blending_type"));

    initialize(c, bridges_.circle());

  } else if (type == "line") {
    data_staging::line l(object_id,
                         counter,
                         vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                         vector2d(i.double_number(instance, "x2"), i.double_number(instance, "y2")),
                         i.double_number(instance, "radiussize"));

    // TODO: no logic for end of line
    l.movement_line_start_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                             i.double_number(instance, "vel_y", 0),
                                             i.double_number(instance, "velocity", 0));

    l.meta_ref().set_namespace(parent_object_ns);

    l.styling_ref().set_blending_type(i.integer_number(instance, "blending_type"));

    initialize(l, bridges_.line());
  } else if (type == "text") {
    data_staging::text t(object_id,
                         counter,
                         vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                         i.str(instance, "text"),
                         i.double_number(instance, "text_size"),
                         i.str(instance, "text_align"),
                         i.boolean(instance, "text_fixed"));

    t.set_font_name(i.has_field(instance, "text_font") ? i.str(instance, "text_font") : "monaco.ttf");

    t.movement_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                  i.double_number(instance, "vel_y", 0),
                                  i.double_number(instance, "velocity", 0));

    t.meta_ref().set_namespace(parent_object_ns);

    t.styling_ref().set_blending_type(i.integer_number(instance, "blending_type"));

    initialize(t, bridges_.text());

  } else if (type == "script") {
    data_staging::script s(
        object_id, counter, vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")));

    s.meta_ref().set_namespace(object_id + "_");

    initialize(s, bridges_.script());
  } else {
    throw std::logic_error(fmt::format("unknown type: {}", type));
  }

  if (!shape_ref) {
    throw std::runtime_error("unexpected shape_ref not set to reference");
  }

  if (check_uniqueness) {
    auto& created_shape_copy = *shape_copy;

    // we cannot trust shape_ref.get() to be valid, since spawning objects can be done recursively, and
    // this can trigger the underlying datastructure to reallocate memory etc. so we need to retrieve it again
    int uid = -1;
    meta_callback(created_shape_copy, [&](const auto& cc) {
      uid = cc.meta_cref().unique_id();
    });
    std::optional<std::reference_wrapper<data_staging::shape_t>> shape_ref_opt;
    for (auto& v : instantiated_objects[scenes_.scenesettings.current_scene_next]) {
      meta_callback(v, [&](const auto& ccc) {
        if (ccc.meta_cref().unique_id() == uid) {
          shape_ref_opt = v;
        }
      });
    }
    auto shape_ref = *shape_ref_opt;

    std::vector<std::reference_wrapper<data_staging::shape_t>> new_stack;
    recursively_build_stack_for_object(
        new_stack, shape_ref.get(), next_instance_map, instantiated_objects[scenes_.scenesettings.current_scene_next]);
    // reverse new_stack
    std::reverse(new_stack.begin(), new_stack.end());

    handle_rotations(shape_ref.get(), new_stack);

    bool destroyed = false;

    // round x to the nearest 0.25 resolution x, y
    // auto& created_instance = instance;

    const auto destroy_shape = [&]() {
      destroy(shape_ref.get());
      destroyed = true;
    };

    meta_visit(
        shape_ref.get(),
        [&](data_staging::circle& c) {
          unique_groups[unique_group].query(destroy_shape,
                                            c.transitive_location_ref().position_ref().x,
                                            c.transitive_location_ref().position_ref().y);
        },
        [&](data_staging::line& l) {
          unique_groups[unique_group].query(destroy_shape,
                                            l.transitive_line_start_ref().position_ref().x,
                                            l.transitive_line_start_ref().position_ref().y,
                                            l.transitive_line_end_ref().position_ref().x,
                                            l.transitive_line_end_ref().position_ref().y);
        },
        [&](data_staging::text& t) {
          unique_groups[unique_group].query(destroy_shape,
                                            t.transitive_location_ref().position_ref().x,
                                            t.transitive_location_ref().position_ref().y);
        },
        [&](data_staging::script& s) {
          unique_groups[unique_group].query(destroy_shape,
                                            s.transitive_location_ref().position_ref().x,
                                            s.transitive_location_ref().position_ref().y);
        });

    if (destroyed) {
      return std::nullopt;
    }
  }
  // TODO: simply return here some boolean whether or not this guy is part of a uniqueness group
  // then the caller can do something like If (is_unique) { ... dedupe logic ...  }
  return std::make_tuple(instance, *shape_ref, *shape_copy);
}
void generator::_instantiate_object(v8_interact& i,
                                    std::optional<v8::Local<v8::Object>> scene_obj,
                                    v8::Local<v8::Object> object_prototype,
                                    v8::Local<v8::Object> new_instance,
                                    int64_t level,
                                    const std::string& namespace_) {
  v8::Isolate* isolate = i.get_isolate();

  i.recursively_copy_object(new_instance, object_prototype);

  if (!namespace_.empty()) {
    i.set_field(new_instance, "namespace", v8_str(i.get_context(), namespace_));
  }

  if (scene_obj) {
    generator::_instantiate_object_copy_fields(i, *scene_obj, new_instance);
  }

  i.set_field(new_instance, "subobj", v8::Array::New(isolate));
  i.set_field(new_instance, "level", v8::Number::New(isolate, level));
  {
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_int_distribution<int> distribution{'a', 'z'};
    static auto generate_len = 6;
    static std::string rand_str(generate_len, '\0');
    for (auto& dis : rand_str) dis = distribution(generator);
    i.set_field(new_instance, "__random_hash__", v8_str(i.get_context(), rand_str));
  }
  i.set_field(new_instance, "__instance__", v8::Boolean::New(isolate, true));

  // Make sure we deep copy the gradients
  i.set_field(new_instance, "gradients", v8::Array::New(isolate));
  auto dest_gradients = i.get(new_instance, "gradients").As<v8::Array>();
  auto gradients = i.has_field(object_prototype, "gradients") && i.get(object_prototype, "gradients")->IsArray()
                       ? i.get(object_prototype, "gradients").As<v8::Array>()
                       : v8::Array::New(i.get_isolate());
  for (size_t k = 0; k < gradients->Length(); k++) {
    i.set_field(dest_gradients, k, v8::Array::New(isolate));

    auto gradient = i.get_index(gradients, k).As<v8::Array>();
    auto dest_gradient = i.get_index(dest_gradients, k).As<v8::Array>();
    for (size_t l = 0; l < gradient->Length(); l++) {
      i.set_field(dest_gradient, l, i.get_index(gradient, l));
    }
  }

  // Ensure we have a props object in the new obj
  if (!i.has_field(new_instance, "props")) {
    i.set_field(new_instance, "props", v8::Object::New(i.get_isolate()));
  }

  // Copy over scene properties to instance properties
  if (scene_obj) {
    auto props = i.v8_obj(new_instance, "props");
    auto scene_props = i.v8_obj(*scene_obj, "props");
    auto obj_fields = i.prop_names(scene_props);

    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_value = i.get(scene_props, field_name);
      i.set_field(props, field_name, field_value);
    }
  }

  auto the_fun = i.get_fun("__spawn__");
  i.set_fun(new_instance, "spawn", the_fun);
}

void generator::_instantiate_object_copy_fields(v8_interact& i,
                                                v8::Local<v8::Object> scene_obj,
                                                v8::Local<v8::Object> new_instance) {
  i.copy_field_if_exists(new_instance, "id", scene_obj);
  i.copy_field_if_exists(new_instance, "x", scene_obj);
  i.copy_field_if_exists(new_instance, "y", scene_obj);
  i.copy_field_if_exists(new_instance, "x2", scene_obj);
  i.copy_field_if_exists(new_instance, "y2", scene_obj);
  i.copy_field_if_exists(new_instance, "vel_x", scene_obj);
  i.copy_field_if_exists(new_instance, "vel_y", scene_obj);
  i.copy_field_if_exists(new_instance, "vel_x2", scene_obj);
  i.copy_field_if_exists(new_instance, "vel_y2", scene_obj);
  i.copy_field_if_exists(new_instance, "velocity", scene_obj);
  i.copy_field_if_exists(new_instance, "mass", scene_obj);
  i.copy_field_if_exists(new_instance, "radius", scene_obj);
  i.copy_field_if_exists(new_instance, "radiussize", scene_obj);
  i.copy_field_if_exists(new_instance, "gradient", scene_obj);
  i.copy_field_if_exists(new_instance, "texture", scene_obj);
  i.copy_field_if_exists(new_instance, "seed", scene_obj);
  i.copy_field_if_exists(new_instance, "blending_type", scene_obj);
  i.copy_field_if_exists(new_instance, "opacity", scene_obj);
  i.copy_field_if_exists(new_instance, "scale", scene_obj);
  i.copy_field_if_exists(new_instance, "angle", scene_obj);
  i.copy_field_if_exists(new_instance, "rotate", scene_obj);
  i.copy_field_if_exists(new_instance, "hue", scene_obj);
  i.copy_field_if_exists(new_instance, "pivot", scene_obj);
  i.copy_field_if_exists(new_instance, "text", scene_obj);
  i.copy_field_if_exists(new_instance, "text_align", scene_obj);
  i.copy_field_if_exists(new_instance, "text_size", scene_obj);
  i.copy_field_if_exists(new_instance, "text_fixed", scene_obj);
  i.copy_field_if_exists(new_instance, "text_font", scene_obj);
  // this function is also used for parent -> child field inheritence.
  // for scripts, the 'file' field should never be inherited.
  // i.copy_field_if_exists(new_instance, "file", scene_obj);
  i.copy_field_if_exists(new_instance, "duration", scene_obj);
  i.copy_field_if_exists(new_instance, "collision_group", scene_obj);
  i.copy_field_if_exists(new_instance, "gravity_group", scene_obj);
  i.copy_field_if_exists(new_instance, "unique_group", scene_obj);
}

void generator::debug_print_next() {
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
                 << ", gradient = " << sty.gradient() << std::endl;
  };
  for (auto& shape : scene_shapes_next[scenes_.scenesettings.current_scene_next]) {
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

template <typename T>
void generator::copy_gradient_from_object_to_shape(
    T& source_object,
    data::shape& destination_shape,
    std::unordered_map<std::string, data::gradient>& known_gradients_map) {
  if constexpr (!std::is_same_v<T, data_staging::script>) {
    std::string namespace_ = source_object.meta_cref().namespace_name();
    std::string gradient_id = namespace_ + source_object.styling_ref().gradient();

    if (!gradient_id.empty()) {
      if (destination_shape.gradients_.empty()) {
        if (known_gradients_map.find(gradient_id) != known_gradients_map.end()) {
          destination_shape.gradients_.emplace_back(1.0, known_gradients_map[gradient_id]);
        }
      }
    }
    for (const auto& [opacity, gradient_id] : source_object.styling_ref().get_gradients_cref()) {
      destination_shape.gradients_.emplace_back(opacity, known_gradients_map[gradient_id]);
    }
  }
}

template <typename T>
void generator::copy_texture_from_object_to_shape(T& source_object,
                                                  data::shape& destination_shape,
                                                  std::unordered_map<std::string, data::texture>& known_textures_map) {
  if constexpr (!std::is_same_v<T, data_staging::script>) {
    std::string namespace_ = source_object.meta_cref().namespace_name();
    std::string texture_id = namespace_ + source_object.styling_ref().texture();

    if (!texture_id.empty()) {
      if (destination_shape.textures.empty()) {
        if (known_textures_map.find(texture_id) != known_textures_map.end()) {
          destination_shape.textures.emplace_back(1.0, known_textures_map[texture_id]);
        }
      }
    }

    for (const auto& [opacity, texture_id] : source_object.styling_ref().get_textures_cref()) {
      destination_shape.textures.emplace_back(opacity, known_textures_map[texture_id]);
    }
  }
}

template <typename T>
void generator::write_back_copy(T& copy) {
  size_t index = 0;
  for (auto& instance : instantiated_objects[scenes_.scenesettings.current_scene_next]) {
    meta_callback(instance, [&]<typename TS>(TS& shape) {
      if (shape.meta_cref().unique_id() == copy.meta_cref().unique_id()) {
        instantiated_objects[scenes_.scenesettings.current_scene_next][index] = copy;
      }
    });
    index++;
  }
}
}  // namespace interpreter
