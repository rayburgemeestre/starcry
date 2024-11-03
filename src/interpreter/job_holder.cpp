/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "job_holder.h"

#include "data/job.hpp"

namespace interpreter {
void job_holder::init() {
  job = std::make_shared<data::job>();
  job->frame_number = 0;
}

std::shared_ptr<data::job> job_holder::get() const {
  return job;
}

std::shared_ptr<data::job>& job_holder::get_ref() {
  return job;
}

}  // namespace interpreter