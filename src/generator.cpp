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
#include "image.hpp"
#include "interpreter/abort_exception.hpp"
#include "interpreter/debug_printer.h"
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
      initializer_(gradient_manager_,
                   texture_manager_,
                   toroidal_manager_,
                   context,
                   metrics,
                   rand_,
                   global_attrs_,
                   settings_,
                   object_lookup_,
                   scenes_,
                   scalesettings,
                   bridges_,
                   sampler_,
                   definitions_,
                   opts,
                   state_,
                   config_),
      spawner_(genctx, definitions_, instantiator_, object_lookup_, scenes_),
      bridges_(definitions_, spawner_),
      scenes_(*this),
      sampler_(*this),
      positioner_(*this),
      interactor_(*this, toroidal_manager_, definitions_, spawner_),
      instantiator_(*this, definitions_, initializer_, object_lookup_),
      job_shape_mapper_(*this, gradient_manager_, texture_manager_),
      object_lookup_(*this),
      checkpoints_(*this),
      debug_printer_(*this),
      generator_opts(opts) {}

void generator::init(const std::string& filename,
                     std::optional<double> rand_seed,
                     bool preview,
                     bool caching,
                     std::optional<int> width,
                     std::optional<int> height,
                     std::optional<double> scale) {
  prctl(PR_SET_NAME, "native generator thread");
  config().filename = filename;
  job = std::make_shared<data::job>();
  job->frame_number = 0;

  initializer_.initialize_all(job, config().filename, rand_seed, preview, width, height, scale);

  context->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    genctx = std::make_shared<generator_context>(val, 0);
  });

  scenes_.initialize();

  // reset random number generator
  rand_.set_seed(state().seed);

  // set_scene requires generator_context to be set
  scenes_.set_scene(0);

  // all objects added at this point can be blindly appended
  scenes_.append_instantiated_objects();

  config().caching = caching;
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

void generator::reset_seeds() {
  instantiator_.reset_seeds();
}

void generator::fast_forward(int frame_of_interest) {
  fast_forwarder(
      config().fast_ff,
      frame_of_interest,
      config().min_intermediates,
      config().max_intermediates,
      [&]() {
        generate_frame();
        metrics_->skip_job(job->job_number);
      },
      [&](int goto_frame) {
        // since we're jumping to a specific frame, we will create a 'fake' entry for the
        // frame before, and mark it as skipped, right now the web interface (TimeLineComponent)
        // uses that to give the correct colors to the individual frames.
        metrics_->register_job(goto_frame - 1, goto_frame - 1, 0, 1);
        metrics_->skip_job(goto_frame - 1);

        // restore everything from cache for this frame.
        if (checkpoints_.get_scenes().contains(goto_frame)) {
          scenes_.load_from(checkpoints_.get_scenes().at(goto_frame));
          std::swap(*job, checkpoints_.job().at(goto_frame));
          object_lookup_.update();
          job->frame_number = goto_frame;
        }
      },
      true,
      checkpoints_.available());
}

bool generator::generate_frame() {
  return sampler_.sample(config().fps, [&](bool skip) {
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
      if (config().caching) {
        checkpoints_.insert(*job, scenes_);
      }

      genctx = std::make_shared<generator_context>(val, scenes_.scenesettings.current_scene_next);
      auto& i = genctx->i();

      auto obj = val.As<v8::Object>();
      auto video = i.v8_obj(obj, "video");
      if (!video->IsObject()) video = v8::Object::New(isolate);

      stepper.reset();
      indexes.clear();
      int attempt = 0;
      double max_dist_found = std::numeric_limits<double>::max();
      scalesettings.reset();

      if (config().min_intermediates > 0.) {
        update_steps(config().min_intermediates);
      }

      static const auto max_attempts = 2;
      while (max_dist_found > config().tolerated_granularity && !stepper.frozen) {
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

          object_lookup_.update();  // object uid -> object ref

          positioner_.update_object_positions();  // velocity + time

          positioner_.update_rotations();  // rotate + absolute x,y

          interactor_.update_interactions();  // toroidal, collisions, gravity, dedupe

          update_object_distances(&attempt, &max_dist_found);  // calculate distance and steps

          // above update functions could have triggered spawning of new objects
          insert_newly_created_objects();

          job_shape_mapper_.convert_objects_to_render_job(sc, video);

          scenes_.commit_scene_shapes_intermediates();

          scalesettings.update();
          scenes_.update();

          if (job->shapes.size() != size_t(stepper.max_step)) {
            detected_too_many_steps = true;
          }
          metrics_->update_steps(job->job_number + 1, attempt, stepper.current_step);
        }
        if (!detected_too_many_steps) {
          // didn't bail out with break above
          if (stepper.max_step == config().max_intermediates) {
            // config doesn't allow finer granularity any way, break.
            break;
          }
          if (stepper.max_step > config().max_intermediates) {
            logger(DEBUG) << "stepper.max_step > max_intermediates -> " << stepper.max_step << " > "
                          << config().max_intermediates << std::endl;
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
        object_lookup_.set_dirty();
      }

      scenes_.commit_scene_shapes();

      scalesettings.commit();
      scenes_.commit();
      // scenes_.memory_dump();
      if (generator_opts.debug) {
        debug_printer_.debug_print_next();
      }

      // cleanup for next iteration
      interactor_.reset();

      metrics_->update_steps(job->job_number, attempt, stepper.current_step);
    });

    metrics_->complete_job(job->job_number);

    job->job_number++;
    job->frame_number++;

    v8::HeapStatistics hs;
    context->isolate->GetHeapStatistics(&hs);
    const auto total_usage = (double(getValue()) / 1024. / 1024.);
    const auto v8_usage = (hs.total_heap_size() / 1024. / 1024. / 1024.);
    logger(INFO) << "Memory usage: " << total_usage << " GB. "
                 << "V8 Heap: " << v8_usage << " GB. "
                 << "Other: " << (total_usage - v8_usage) << " GB." << std::endl;
    image_repository::instance().print();
    fps_progress_.inc();
  } catch (abort_exception& ex) {
    std::cout << "[caught] " << ex.what() << " (abort)" << std::endl;
    std::exit(0);
  } catch (std::exception& ex) {
    std::cout << "[caught] " << ex.what() << std::endl;
  }
  job->last_frame = job->frame_number == state().max_frames;
  return job->frame_number != state().max_frames;
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
        }
        if (uid == parent_uid) {
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
  object_lookup_.update();
}

void generator::update_object_distances(int* attempt, double* max_dist_found) {
  stepper.reset_current();
  const auto handle = [&](data_staging::shape_t& abstract_shape, data_staging::meta& meta) {
    const auto instance_uid = meta.unique_id();
    const auto find = object_lookup_.find_intermediate(instance_uid);
    if (find == object_lookup_.end_intermediate()) {
      return;
    }
    const double dist = get_max_travel_of_object(abstract_shape, find->second.get());
    if (dist > *max_dist_found) {
      *max_dist_found = dist;
    }
    const auto steps = update_steps(dist);

    static std::unordered_map<int64_t, int> recorded_steps;

    meta.set_distance(*max_dist_found);
    if (*attempt == 1) {
      meta.set_steps(steps);
      recorded_steps[instance_uid] = steps;
    } else if (*attempt > 1) {
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
    if (const auto find = definitions_.get(instance_id, true); find) {
      const auto object_definition = *find;
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
      meta_callback(instance, [&](auto& shape) {
        handle_time_for_shape(shape, bridges_.get<std::decay_t<decltype(shape)>>());
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
  auto steps = round(std::max(1.0, fabs(dist) / config().tolerated_granularity));
  if (steps > config().max_intermediates) {
    steps = config().max_intermediates;
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

  auto process_shape = []<typename XY, typename XY2, typename Radius>(
                           auto& shape, XY& xy, XY2* xy2, Radius* radius, bool* compare_xy2) {
    if constexpr (std::is_same_v<std::decay_t<decltype(shape)>, data_staging::circle>) {
      xy = shape.transitive_location_ref().position_cref();
      if (radius) *radius = shape.radius() + shape.radius_size();
    } else if constexpr (std::is_same_v<std::decay_t<decltype(shape)>, data_staging::ellipse>) {
      xy = shape.transitive_location_ref().position_cref();
      // ... (add any ellipse-specific logic here)
    } else if constexpr (std::is_same_v<std::decay_t<decltype(shape)>, data_staging::line>) {
      xy = shape.transitive_line_start_ref().position_cref();
      if (xy2) *xy2 = shape.transitive_line_end_ref().position_cref();
      if (compare_xy2) *compare_xy2 = true;
    } else if constexpr (std::is_same_v<std::decay_t<decltype(shape)>, data_staging::text> ||
                         std::is_same_v<std::decay_t<decltype(shape)>, data_staging::script>) {
      xy = shape.transitive_location_ref().position_cref();
    } else {
      static_assert(!sizeof(shape), "Unsupported shape type");
    }
  };

  meta_callback(shape_now, [&](auto& shape) {
    process_shape(shape, xy_now, &xy2_now, &radius_now, &compare_xy2);
  });

  meta_callback(shape_prev, [&](auto& shape) {
    process_shape(shape, xy_prev, &xy2_prev, &radius_prev, nullptr);
  });

  auto dist = xy_now.distance_to(xy_prev);
  if (compare_xy2) dist = std::max(dist, xy2_now.distance_to(xy2_prev));

  dist += squared_dist(radius_now, radius_prev);

  return dist;
}

std::shared_ptr<data::job> generator::get_job() const {
  return job;
}

data::settings generator::settings() const {
  return settings_;
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

std::vector<int64_t> generator::get_transitive_ids(const std::vector<int64_t>& unique_ids) {
  std::vector<int64_t> ret = unique_ids;
  object_lookup_.update_if_dirty();
  try {
    auto queue = unique_ids;
    while (!queue.empty()) {
      const auto item = queue.back();
      queue.pop_back();
      const auto obj = object_lookup_.at(item).get();
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

void generator::set_checkpoints(std::set<int>& checkpoints) {
  checkpoints_.set_checkpoints(checkpoints);
}

generator_state& generator::state() {
  return state_;
}

generator_config& generator::config() {
  return config_;
}

}  // namespace interpreter
