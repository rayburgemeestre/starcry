/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "generator.h"

#include <linux/prctl.h>
#include <sys/prctl.h>
#include <cmath>
#include <memory>

#define BOOST_DI_CFG_CTOR_LIMIT_SIZE 30
#include "boost/di.hpp"

#include "v8pp/module.hpp"

#include "core/delayed_exit.hpp"
#include "image.hpp"
#include "interpreter/abort_exception.hpp"
#include "interpreter/debug_printer.h"
#include "interpreter/fast_forwarder.hpp"
#include "interpreter/gradient_manager.h"
#include "interpreter/texture_manager.h"
#include "interpreter/toroidal_manager.h"
#include "starcry/metrics.h"
#include "util/benchmark.h"
#include "util/generator_context.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/memory_analyzer.hpp"
#include "util/memory_usage.hpp"
#include "util/step_calculator.hpp"
#include "util/vector_logic.hpp"

namespace interpreter {
generator::generator(std::shared_ptr<metrics> metrics,
                     std::shared_ptr<v8_wrapper> context,
                     generator_context_wrapper& genctx,
                     generator_state& state,
                     generator_config& config,
                     const generator_options& opts,
                     job_holder& job_holder,
                     frame_stepper& stepper,
                     util::random_generator& rand,
                     data::settings& settings,
                     scale_settings& scalesettings,
                     frame_sampler& sampler,
                     initializer& initializer,
                     spawner& spawner,
                     bridges& bridges,
                     scenes& scenes,
                     positioner& positioner,
                     interactor& interactor,
                     instantiator& instantiator,
                     job_to_shape_mapper& job_shape_mapper,
                     object_lookup& objectlookup,
                     debug_printer& debug_printer,
                     std::shared_ptr<Benchmark> benchmark)
    : state_(state),
      config_(config),
      context(context),
      metrics_(metrics),
      genctx(genctx),
      job_holder_(job_holder),
      stepper(stepper),
      settings_(settings),
      scalesettings_(scalesettings),
      initializer_(initializer),
      spawner_(spawner),
      bridges_(bridges),
      scenes_(scenes),
      sampler_(sampler),
      positioner_(positioner),
      interactor_(interactor),
      instantiator_(instantiator),
      job_shape_mapper_(job_shape_mapper),
      object_lookup_(objectlookup),
      // checkpoints_(*this),
      debug_printer_(debug_printer),
      rand_(rand),
      generator_opts(opts),
      benchmark_(benchmark) {
  instantiator.init(interactor_);
}

std::shared_ptr<generator> generator::create(std::shared_ptr<metrics> metrics__,
                                             std::shared_ptr<v8_wrapper> context__,
                                             generator_options& opts__,
                                             generator_state& state__,
                                             generator_config& config__,
                                             std::shared_ptr<Benchmark> benchmark__) {
  namespace di = boost::di;
  auto genctx = std::make_shared<generator_context>();
  auto injector = di::make_injector(di::bind<metrics>().to(metrics__),
                                    di::bind<v8_wrapper>().to(context__),
                                    di::bind<generator_options>().to(opts__),
                                    di::bind<generator_context>().to(genctx),
                                    di::bind<generator_state>().to(state__),
                                    di::bind<generator_config>().to(config__),
                                    di::bind<Benchmark>().to(benchmark__));
  return injector.create<std::unique_ptr<generator>>();
}

void generator::init(const std::string& filename,
                     std::optional<double> rand_seed,
                     bool preview,
                     bool caching,
                     std::optional<int> width,
                     std::optional<int> height,
                     std::optional<double> scale) {
  prctl(PR_SET_NAME, "native generator thread");
  config_.filename = filename;
  job_holder_.init();

  initializer_.initialize_all(
      job_holder_.get(), config_.filename, rand_seed, preview, width, height, scale, scenes_, spawner_);

  context->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    genctx.init(val, 0);
  });

  scenes_.initialize();

  // reset random number generator
  rand_.set_seed(state_.seed);

  // set_scene requires generator_context to be set
  scenes_.set_scene(0, instantiator_);

  // all objects added at this point can be blindly appended
  scenes_.append_instantiated_objects();

  config_.caching = caching;
}

void generator::reset_seeds() {
  instantiator_.reset_seeds();
}

void generator::fast_forward(int frame_of_interest) {
  fast_forwarder(
      config_.fast_ff,
      frame_of_interest,
      config_.min_intermediates,
      config_.max_intermediates,
      [&]() {
        generate_frame();
        metrics_->skip_job(job_holder_.get()->job_number);
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
          std::swap(*job_holder_.get(), checkpoints_.job().at(goto_frame));
          object_lookup_.update();
          job_holder_.get()->frame_number = goto_frame;
        }
      },
      true,
      checkpoints_.available());
}

bool generator::generate_frame() {
  return sampler_.sample(config_.fps, [&](bool skip) {
    auto ret = _generate_frame();
    if (skip) job_holder_.get()->job_number--;
    return ret;
  });
}

bool generator::_generate_frame() {
  delayed_exit de(10);
  // TODO: this measures the generation of the frame only
  // TODO: add support for multiple rendering things
  const auto start = benchmark_ ? benchmark_->measure("frame generation") : std::chrono::high_resolution_clock::now();
  try {
    job_holder_.get()->shapes.clear();

    // job_number is incremented later, hence we do a +1 on the next line.
    metrics_->register_job(job_holder_.get()->job_number + 1,
                           job_holder_.get()->frame_number,
                           job_holder_.get()->chunk,
                           job_holder_.get()->num_chunks);

    context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
      if (config_.caching) {
        checkpoints_.insert(*job_holder_.get(), scenes_);
      }

      genctx.init(val, scenes_.scenesettings.current_scene_next);
      auto& i = genctx.get()->i();

      auto obj = val.As<v8::Object>();
      auto video = i.v8_obj(obj, "video");
      if (!video->IsObject()) video = v8::Object::New(isolate);

      stepper.reset();
      job_shape_mapper_.reset();
      int attempt = 0;
      double max_dist_found = std::numeric_limits<double>::max();
      scalesettings_.reset();

      if (config_.min_intermediates > 0.) {
        update_steps(config_.min_intermediates);
      }

      static const auto max_attempts = 2;
      while (max_dist_found > config_.tolerated_granularity && !stepper.frozen) {
        if (++attempt >= max_attempts) {
          stepper.freeze();
        }
        // logger(DEBUG) << "Generating frame [native] " << job_holder_.get()->frame_number << " attempt " << attempt <<
        // std::endl;
        max_dist_found = 0;
        if (attempt > 1) {
          if (!settings_.motion_blur) break;
          revert_all_changes(i);
        }
        step_calculator sc(stepper.max_step);
        job_holder_.get()->resize_for_num_steps(stepper.max_step);
        metrics_->set_steps(job_holder_.get()->job_number + 1, attempt, stepper.max_step);

        stepper.rewind();
        bool detected_too_many_steps = false;
        while (stepper.has_next_step() && !detected_too_many_steps) {
          // logger(DEBUG) << "Stepper at step " << frame_stepper_.current_step << " out of " << frame_stepper_.max_step
          // << std::endl;
          stepper.advance_step();

          interactor_.reset();

          scenes_.prepare_scene(instantiator_);

          object_lookup_.update();  // object uid -> object ref

          positioner_.update_object_positions();  // velocity + time

          positioner_.update_rotations();  // rotate + absolute x,y

          interactor_.update_interactions();  // toroidal, collisions, gravity, dedupe

          update_object_distances(&attempt, &max_dist_found);  // calculate distance and steps

          // above update functions could have triggered spawning of new objects
          insert_newly_created_objects();

          job_shape_mapper_.convert_objects_to_render_job(sc, video);

          scenes_.commit_scene_shapes_intermediates();

          scalesettings_.update();
          scenes_.update();

          if (job_holder_.get()->shapes.size() != size_t(stepper.max_step)) {
            detected_too_many_steps = true;
          }
          metrics_->update_steps(job_holder_.get()->job_number + 1, attempt, stepper.current_step);
        }
        if (!detected_too_many_steps) {
          // didn't bail out with break above
          if (stepper.max_step == config_.max_intermediates) {
            // config doesn't allow finer granularity any way, break.
            break;
          }
          if (stepper.max_step > config_.max_intermediates) {
            logger(DEBUG) << "frame_stepper_.max_step > max_intermediates -> " << stepper.max_step << " > "
                          << config_.max_intermediates << std::endl;
            throw abort_exception("frame_stepper_ max exceeds max. intermediates");
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

      scalesettings_.commit();
      scenes_.commit();
      // scenes_.memory_dump();
      if (generator_opts.debug) {
        debug_printer_.debug_print_next();
      }

      // cleanup for next iteration
      interactor_.reset();

      metrics_->update_steps(job_holder_.get()->job_number, attempt, stepper.current_step);
    });

    metrics_->complete_job(job_holder_.get()->job_number);
    if (benchmark_) benchmark_->store("frame generation", start);
    job_holder_.get()->job_number++;
    job_holder_.get()->frame_number++;

    v8::HeapStatistics hs;
    context->isolate->GetHeapStatistics(&hs);
    const auto total_usage = (double(getValue()) / 1024. / 1024.);
    const auto v8_usage = (hs.total_heap_size() / 1024. / 1024. / 1024.);
    logger(INFO) << "Memory usage: " << total_usage << " GB. "
                 << "V8 Heap: " << v8_usage << " GB. "
                 << "Other: " << (total_usage - v8_usage) << " GB." << std::endl;
    image_repository::instance().print();
    // memory_analyzer::print_report();
    fps_progress_.inc();
  } catch (abort_exception& ex) {
    std::cout << "[caught] " << ex.what() << " (abort)" << std::endl;
    std::exit(0);
  } catch (std::exception& ex) {
    std::cout << "[caught] " << ex.what() << std::endl;
  }
  job_holder_.get()->last_frame = job_holder_.get()->frame_number == state_.max_frames;
  return job_holder_.get()->frame_number != state_.max_frames;
}

void generator::revert_all_changes(v8_interact& i) {
  job_holder_.get()->shapes.clear();
  job_shape_mapper_.reset();

  // reset next and intermediate instances
  scenes_.reset_scene_shapes_next();
  scenes_.reset_scene_shapes_intermediates();

  scalesettings_.revert();
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

int generator::update_steps(double dist) {
  auto steps = round(std::max(1.0, fabs(dist) / config_.tolerated_granularity));
  if (steps > config_.max_intermediates) {
    steps = config_.max_intermediates;
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
  return job_holder_.get();
}

data::settings generator::settings() const {
  return settings_;
}

v8::Local<v8::Value> generator::get_attr(data_staging::shape_t& spawner, v8::Local<v8::String> field) {
  auto& i = genctx.get()->i();
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

std::string generator::get_js_api() {
  return initializer_.get_js_api();
}

}  // namespace interpreter
