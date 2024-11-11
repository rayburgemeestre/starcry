/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <unordered_map>

#include "data_staging/shape.hpp"
#include "scenesettings.h"
#include "util/generator_context.h"

class generator_context;
class frame_stepper;
struct generator_state;
struct generator_config;

namespace interpreter {

struct time_settings {
  double time;
  double elapsed;
  double scene_time;
};

class generator;
class instantiator;
class job_holder;

class scenes : public transaction {
  // TODO: refs to shared ptrs
  std::shared_ptr<v8_wrapper> context;
  generator_context_wrapper& genctx;
  instantiator& instantiator_;
  frame_stepper& stepper;
  job_holder& job_holder_;
  generator_state& state_;
  generator_config& config_;

  std::vector<std::vector<data_staging::shape_t>> scene_shapes;
  std::vector<std::vector<data_staging::shape_t>> scene_shapes_next;
  std::vector<std::vector<data_staging::shape_t>> instantiated_objects;
  std::vector<std::vector<data_staging::shape_t>> scene_shapes_intermediate;

public:  // temporary
  scene_settings scenesettings;
  std::unordered_map<int64_t, scene_settings> scenesettings_objs;

public:
  explicit scenes(std::shared_ptr<v8_wrapper> context,
                  generator_context_wrapper& genctx,
                  instantiator& instantiator,
                  frame_stepper& stepper,
                  job_holder& job_holder,
                  generator_state& state,
                  generator_config& config);
  scenes(const scenes& other) = default;

  void load_from(const scenes& other);
  scenes clone();

  void dump();
  void memory_dump();
  void initialize();
  void add_scene();
  void append_instantiated_objects();
  void set_scene(size_t scene);
  void switch_scene();
  void prepare_scene();

  // for scene settings
  void update();
  void commit() override;
  void revert() override;
  void reset() override;

  // for scene objects
  void commit_scene_shapes();
  void commit_scene_shapes_intermediates();
  void reset_scene_shapes_next();
  void reset_scene_shapes_intermediates();
  [[nodiscard]] bool cleanup_destroyed_objects();
  std::vector<data_staging::shape_t>& shapes_current_scene();
  std::vector<data_staging::shape_t>& next_shapes_current_scene();
  std::vector<data_staging::shape_t>& intermediate_shapes_current_scene();
  std::vector<data_staging::shape_t>& instantiated_objects_current_scene();

  double get_duration(int64_t unique_id);
  void set_duration(int64_t unique_id, double duration);
  void set_durations(int64_t unique_id, std::vector<double>& durations);
  void set_desired_duration(int64_t unique_id, double value);

  time_settings get_time(scene_settings& settings) const;

  size_t current();
  void set_scene_sub_object(int64_t unique_id);

private:
  void _set_scene_sub_object(scene_settings& scenesettings, size_t scene);
  void refresh_scenesettings();
  void create_object_instances();
};

}  // namespace interpreter
