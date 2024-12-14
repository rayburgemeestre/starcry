/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <cstddef>
#include <cstdint>

#include "scalesettings.h"
#include "scenesettings.h"

#include "interpreter/bridges.h"
#include "interpreter/checkpoints.h"
#include "interpreter/debug_printer.h"
#include "interpreter/frame_sampler.h"
#include "interpreter/generator_options.hpp"
#include "interpreter/initializer.h"
#include "interpreter/instantiator.h"
#include "interpreter/interactor.h"
#include "interpreter/job_holder.h"
#include "interpreter/job_to_shape_mapper.h"
#include "interpreter/object_lookup.h"
#include "interpreter/positioner.h"
#include "interpreter/scenes.h"
#include "interpreter/spawner.h"

#include "core/fps_progress.hpp"

#include "data/job.hpp"
#include "data/settings.hpp"
#include "data_staging/shape.hpp"

#include "util/frame_stepper.hpp"
#include "util/generator_context.h"
#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"

class v8_wrapper;
class step_calculator;
class metrics;
class vector2d;
class generator_context_wrapper;
class Benchmark;

namespace util {
class random_generator;
}

namespace interpreter {

class object_definitions;
class gradient_manager;
class texture_manager;
class toroidal_manager;

class generator {
  generator_state& state_;
  generator_config& config_;

  std::shared_ptr<v8_wrapper> context;
  std::shared_ptr<metrics> metrics_;
  generator_context_wrapper& genctx;

  job_holder& job_holder_;

  frame_stepper& stepper;

  data::settings& settings_;
  scale_settings& scalesettings_;

  initializer& initializer_;
  spawner& spawner_;
  bridges& bridges_;
  scenes& scenes_;
  frame_sampler& sampler_;
  positioner& positioner_;
  interactor& interactor_;
  instantiator& instantiator_;
  job_to_shape_mapper& job_shape_mapper_;
  object_lookup& object_lookup_;
  checkpoints checkpoints_;

  debug_printer& debug_printer_;
  util::random_generator& rand_;
  std::vector<int64_t> selected_ids_;  // TODO: not found???

  const generator_options& generator_opts;
  fps_progress fps_progress_;
  std::shared_ptr<Benchmark> benchmark_ = nullptr;

public:
  explicit generator(std::shared_ptr<metrics> metrics,
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
                     std::shared_ptr<Benchmark> benchmark);
  ~generator() = default;
  static std::shared_ptr<generator> create(std::shared_ptr<metrics> metrics,
                                           std::shared_ptr<v8_wrapper> context,
                                           generator_options& opts,
                                           generator_state& state__,
                                           generator_config& config__,
                                           std::shared_ptr<Benchmark> benchmark__);

  void init(const std::string& filename,
            std::optional<double> rand_seed,
            bool preview,
            bool caching,
            std::optional<int> width = std::nullopt,
            std::optional<int> height = std::nullopt,
            std::optional<double> scale = std::nullopt);

  void reset_seeds();
  void fast_forward(int frame_of_interest);
  bool generate_frame();
  void revert_all_changes(v8_interact& i);
  void insert_newly_created_objects();
  void update_object_distances(int* attempt, double* max_dist_found);
  int update_steps(double dist);
  static double get_max_travel_of_object(data_staging::shape_t& shape_now, data_staging::shape_t& shape_prev);

  std::shared_ptr<data::job> get_job() const;

  data::settings settings() const;

  // TODO: remove, or move to spawner?
  v8::Local<v8::Value> get_attr(data_staging::shape_t& spawner, v8::Local<v8::String> field);

  std::vector<int64_t> get_transitive_ids(const std::vector<int64_t>& in);
  void set_checkpoints(std::set<int>& checkpoints);
  std::string get_js_api();

private:
  bool _generate_frame();
};

}  // namespace interpreter