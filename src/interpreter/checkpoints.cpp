/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "checkpoints.h"
#include "generator.h"

namespace interpreter {

checkpoints::checkpoints(generator& gen) {}

void checkpoints::set_checkpoints(std::set<int>& checkpoints) {
  checkpoints_ = checkpoints;
}

std::map<int, scenes>& checkpoints::get_scenes() {
  return checkpoint_data_scenes_;
}

std::map<int, data::job>& checkpoints::job() {
  return checkpoint_data_job_;
}

std::set<int>& checkpoints::defined() {
  return checkpoints_;
}

std::set<int>& checkpoints::available() {
  return checkpoints_available_;
}

void checkpoints::insert(data::job& job, interpreter::scenes& scenes) {
  if (checkpoints_.contains(job.frame_number) && !checkpoint_data_scenes_.contains(job.frame_number)) {
    // checkpoint this one
    auto cloned = scenes.clone();
    checkpoint_data_scenes_.emplace(job.frame_number, std::move(cloned));
    checkpoint_data_job_.emplace(job.frame_number, job);
    checkpoints_available_.insert(job.frame_number);
  }
}

}  // namespace interpreter
