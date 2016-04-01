#pragma once

#include "common.h"

behavior stdin_reader(event_based_actor *self, const caf::actor &job_generator, const caf::actor &job_storage);
