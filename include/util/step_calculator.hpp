/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

class step_calculator {
private:
  std::unordered_map<int, std::unordered_map<int, bool>> step_map;

public:
  step_calculator(int max_step) {
    for (int i = 1; i <= max_step; i++) {
      int diff = max_step / i;
      int frame = max_step;
      step_map[i][frame] = true;
      for (int j = 1; j < i; j++) {
        frame = max_step - (diff * j);
        step_map[i][frame] = true;
      }
    }
  }
  bool do_step(int step_value, int current_step) {
    const auto& step_map_c = step_map;
    if (step_map_c.find(step_value) != step_map_c.end()) {
      const auto val = step_map_c.at(step_value).find(current_step);
      if (val != step_map_c.at(step_value).end()) {
        return val->second;
      }
    }
    return false;
  };
};
