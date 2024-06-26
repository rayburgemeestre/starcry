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
#include "interpreter/frame_sampler.h"
#include "interpreter/generator_options.hpp"
#include "interpreter/initializer.h"
#include "interpreter/instantiator.h"
#include "interpreter/interactor.h"
#include "interpreter/job_mapper.h"
#include "interpreter/object_lookup.h"
#include "interpreter/positioner.h"
#include "interpreter/scenes.h"

#include "core/fps_progress.hpp"

#include "data/job.hpp"
#include "data/settings.hpp"
#include "data/texture.hpp"
#include "data/toroidal.hpp"
#include "data_staging/shape.hpp"

#include "util/frame_stepper.hpp"
#include "util/generator_context.h"
#include "util/random.hpp"
#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"

class v8_wrapper;
class step_calculator;
class metrics;
class vector2d;

namespace interpreter {

class generator {
  friend class initializer;
  friend class bridges;
  friend class scenes;
  friend class frame_sampler;
  friend class positioner;
  friend class interactor;
  friend class instantiator;
  friend class job_mapper;
  friend class object_lookup;
  friend class checkpoints;

  std::shared_ptr<v8_wrapper> context;
  std::shared_ptr<metrics> metrics_;
  std::shared_ptr<data::job> job;
  std::vector<std::reference_wrapper<data_staging::shape_t>> stack;

  uint32_t frame_number = 0;

  size_t max_frames = 0;
  int32_t canvas_w = 0;
  int32_t canvas_h = 0;
  double seed = 1;
  double tolerated_granularity = 1;
  bool minimize_steps_per_object = true;
  size_t use_fps = 25;
  std::unordered_map<std::string, data::gradient> gradients;
  std::unordered_map<std::string, data::texture> textures;
  std::unordered_map<std::string, data::toroidal> toroidals;
  std::unordered_map<size_t, std::map<int, size_t>> indexes;
  frame_stepper stepper;
  int attempt = 0;
  double max_dist_found = std::numeric_limits<double>::max();

  std::unordered_map<std::string, v8::Persistent<v8::Object>> object_definitions_map;
  data::settings settings_;

  int min_intermediates = 1.;
  int max_intermediates = 30.;
  bool fast_ff = false;

  scale_settings scalesettings;

  std::string filename_;

  std::shared_ptr<generator_context> genctx;
  bool debug_;

  initializer initializer_;
  bridges bridges_;
  scenes scenes_;
  frame_sampler sampler_;
  positioner positioner_;
  interactor interactor_;
  instantiator instantiator_;
  job_mapper job_mapper_;
  object_lookup object_lookup_;
  checkpoints checkpoints_;

  util::random_generator rand_;
  data_staging::attrs global_attrs_;
  std::vector<int64_t> selected_ids_;

  const generator_options& generator_opts;

  bool caching_ = false;
  fps_progress fps_progress_;

public:
  explicit generator(std::shared_ptr<metrics>& metrics,
                     std::shared_ptr<v8_wrapper>& context,
                     const generator_options& opts);
  ~generator() = default;

  void init(const std::string& filename,
            std::optional<double> rand_seed,
            bool preview,
            bool caching,
            std::optional<int> width = std::nullopt,
            std::optional<int> height = std::nullopt,
            std::optional<double> scale = std::nullopt);

  void create_object_instances();

  void create_bookkeeping_for_script_objects(v8::Local<v8::Object> created_instance,
                                             const data_staging::shape_t& created_shape,
                                             int debug_level = 0);
  void reset_seeds();
  void fast_forward(int frame_of_interest);
  bool generate_frame();
  void revert_all_changes(v8_interact& i);
  void create_new_mappings();
  void insert_newly_created_objects();
  void update_object_distances();
  void update_time(data_staging::shape_t& object_bridge, const std::string& instance_id, scene_settings& scenesettings);
  int update_steps(double dist);
  static double get_max_travel_of_object(data_staging::shape_t& shape_now, data_staging::shape_t& shape_prev);

  std::shared_ptr<data::job> get_job() const;

  double fps() const;
  int32_t width() const;
  int32_t height() const;
  double get_seed() const;
  data::settings settings() const;
  std::string filename() const;

  v8::Local<v8::Value> get_attr(data_staging::shape_t& spawner, v8::Local<v8::String> field);

  int64_t spawn_object(data_staging::shape_t& spawner, v8::Local<v8::Object> obj);
  int64_t spawn_object2(data_staging::shape_t& spawner, v8::Local<v8::Object> line_obj, int64_t obj1);
  int64_t spawn_object3(data_staging::shape_t& spawner, v8::Local<v8::Object> line_obj, int64_t obj1, int64_t obj2);
  int64_t spawn_object_at_parent(data_staging::shape_t& spawner, v8::Local<v8::Object> obj);
  static int64_t destroy(data_staging::shape_t& spawner);
  v8::Local<v8::Object> get_object(int64_t object_unique_id);

  std::unordered_map<std::string, v8::Persistent<v8::Object>>& get_object_definitions_ref();
  std::vector<int64_t> get_transitive_ids(const std::vector<int64_t>& in);
  void set_checkpoints(std::set<int>& checkpoints);

private:
  bool _generate_frame();

  void debug_print_all();
  void debug_print_next();
  void debug_print(std::vector<data_staging::shape_t>& shapes);
};

}  // namespace interpreter