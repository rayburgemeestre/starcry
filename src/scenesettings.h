/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <limits>
#include <vector>

#include "util/transaction.h"

class scene_settings : public transaction {
public:
  scene_settings() = default;
  virtual ~scene_settings() = default;

  void reset() override;
  bool update(double t);  // override;
  void update();          // override;
  void revert() override;
  void commit() override;

  size_t current_scene = 0;
  size_t current_scene_next = 0;
  size_t current_scene_intermediate = std::numeric_limits<size_t>::max();
  size_t scene_initialized = std::numeric_limits<size_t>::max();
  double scenes_duration = 0;           // non-relative seconds of duration
  double desired_duration = -1;         // in seconds
  std::vector<double> scene_durations;  // normalized
  double offset = 0;
  double offset_next = 0;
  double offset_intermediate = 0;

  double parent_offset = -1;
};
