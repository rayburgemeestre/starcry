/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <unordered_map>

#include "data_staging/shape.hpp"
#include "scenesettings.h"

namespace interpreter {

struct time_settings {
  double time;
  double elapsed;
  double scene_time;
};

class generator;
class scenes : public transaction {
private:
  generator& gen_;

  std::vector<std::vector<data_staging::shape_t>> scene_shapes;
  std::vector<std::vector<data_staging::shape_t>> scene_shapes_next;
  std::vector<std::vector<data_staging::shape_t>> instantiated_objects;
  std::vector<std::vector<data_staging::shape_t>> scene_shapes_intermediate;

public:
  scene_settings scenesettings;
  std::unordered_map<int64_t, scene_settings> scenesettings_objs;

public:
  scenes(generator& gen);

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
  void cleanup_destroyed_objects();
  std::vector<data_staging::shape_t>& next_shapes_current_scene();
  std::vector<data_staging::shape_t>& intermediate_shapes_current_scene();
  std::vector<data_staging::shape_t>& instantiated_objects_current_scene();

  double get_duration(int64_t unique_id);
  void set_duration(int64_t unique_id, double duration);
  void set_durations(int64_t unique_id, std::vector<double>& durations);
  void set_desired_duration(int64_t unique_id, double value);

  interpreter::time_settings get_time(scene_settings& settings) const;

  void set_scene_sub_object(int64_t unique_id);
  void set_scene_sub_object(scene_settings& unique_id, size_t scene);
  size_t current();

private:
  void refresh_scenesettings();
};

}  // namespace interpreter