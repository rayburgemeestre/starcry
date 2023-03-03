/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <unordered_map>

class step_calculator {
private:
  std::unordered_map<int, std::unordered_map<int, bool>> step_map;

public:
  explicit step_calculator(int max_step) {
    for (int i = 1; i <= max_step; i++) {
      double diff = max_step / (double)i;
      int frame = max_step;
      step_map[i][frame] = true;
      int half = max_step / 2.;
      if (i <= half || true) {
        for (int j = 1; j < i; j++) {
          frame = max_step - (diff * j);
          step_map[i][frame] = true;
        }
      } else {
        /**
         * TODO: produce something smoother in this else-branch, to avoid this:
         *  0 = 0 0 0 0 0 0 0 0 0 0, count = 0
         *  1 = 0 0 0 0 0 0 0 0 0 1, count = 1
         *  2 = 0 0 0 0 1 0 0 0 0 1, count = 2
         *  3 = 0 0 1 0 0 1 0 0 0 1, count = 3
         *  4 = 0 1 0 0 1 0 1 0 0 1, count = 4
         *  5 = 0 1 0 1 0 1 0 1 0 1, count = 5 <<<< (3) basically until ~half the indexes, the patterns
         *  6 = 1 0 1 0 1 1 0 1 0 1, count = 6          look okay, until zeros start to be unevenly
         *  7 = 1 1 0 1 1 0 1 1 0 1, count = 7          distributed..
         *  8 = 1 1 1 0 1 1 1 1 0 1, count = 8 <<<< (2) and these shifted a bit..
         *  9 = 1 1 1 1 1 1 1 1 0 1, count = 9 <<<< (1) this zero might be nicer to have in the center
         *  10 = 1 1 1 1 1 1 1 1 1 1, count = 10
         */
      }
    }
  }
  bool do_step(int step_value, int current_step) {
    step_value = std::min((int)step_map.size(), step_value);
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
