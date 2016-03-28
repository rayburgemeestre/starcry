#pragma once

#include "common.h"

behavior job_generator(event_based_actor *self, const caf::actor &job_storage, uint32_t canvas_w, uint32_t canvas_h);
