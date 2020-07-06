/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#include "common.h"

struct job_generator_data;

#include "data/job.hpp"
#include "job_cache.h"

/**
 * Small helper class to job_generator which needs to be global, so some global functions
 *  which are added to the V8 context can also access some of the stuff put in here by
 *  the job_generator actor.
 * This class that sits somewhat between V8 and job_generator is implemented in job_generator.cpp
 */
class generator;
class assistant_ {
public:
  assistant_();

  data::job the_job;
  data::job the_previous_job;
  stateful_actor<job_generator_data> *job_generator;
  generator *generator;
  size_t max_frames;
  bool realtime = false;

  size_t current_frame = 0;
  size_t current_job = 0;
  std::unique_ptr<job_cache> cache;
};

extern std::unique_ptr<assistant_> assistant;
