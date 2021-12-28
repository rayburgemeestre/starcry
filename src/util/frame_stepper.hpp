/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

class frame_stepper {
public:
  int max_step = 1;
  int current_step_max = std::numeric_limits<int>::max();
  int current_step = 0;
  int next_step = 0;

  frame_stepper() = default;

  void reset() {
    max_step = 1;
    current_step_max = std::numeric_limits<int>::max();
    current_step = 0;
    next_step = 0;
  }

  void reset_current() {
    current_step_max = 1;
  }

  void update(int steps) {
    max_step = std::max(max_step, steps);
    current_step_max = std::max(current_step_max, steps);
  }

  void multiply(double multiplier) {
    const auto add = std::max(1.0, static_cast<double>(max_step) * multiplier);
    max_step += add;
  }

  void rewind() {
    current_step = 0;
    next_step = 0;
  }

  bool has_next_step() {
    current_step = next_step;
    bool ret = current_step < max_step;
    next_step++;
    return ret;
  }
};
