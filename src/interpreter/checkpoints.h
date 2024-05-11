/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <set>

#include "scenes.h"

namespace data {
struct job;
}

namespace interpreter {
class generator;

class checkpoints {
public:
  explicit checkpoints(generator& gen);
  void set_checkpoints(std::set<int>& checkpoints);
  std::map<int, scenes>& get_scenes();
  std::map<int, data::job>& job();
  std::set<int>& defined();
  std::set<int>& available();
  void insert(data::job& job, interpreter::scenes& scenes);

private:
  // generator& gen_;

  std::set<int> checkpoints_;
  std::set<int> checkpoints_available_;
  std::map<int, interpreter::scenes> checkpoint_data_scenes_;
  std::map<int, data::job> checkpoint_data_job_;
};

}  // namespace interpreter