/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "common.h"

behavior job_generator(event_based_actor *self, const caf::actor &job_storage, const std::string &filename, uint32_t canvas_w, uint32_t canvas_h, bool use_stdin = false);
