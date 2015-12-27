#pragma once

#include "common.h"

behavior job_generator_idle(event_based_actor* self, const caf::actor &job_storage);
behavior job_generator(event_based_actor* self, const caf::actor &job_storage);
