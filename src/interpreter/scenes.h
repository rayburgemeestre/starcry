/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <unordered_map>

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

public:
  scene_settings scenesettings;
  std::unordered_map<int64_t, scene_settings> scenesettings_objs;

public:
  scenes(generator& gen);

  void initialize();
  void append_instantiated_objects();
  void set_scene(size_t scene);
  void switch_scene();
  void prepare_scene();

  void update();
  void commit() override;
  void revert() override;
  void reset() override;

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