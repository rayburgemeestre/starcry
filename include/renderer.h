#pragma once
#include "common.h"

using start_rendering = atom_constant<atom("startrend")>;
using get_job_atom = atom_constant<atom("getjob")>;
using remove_job_atom = atom_constant<atom("removejob")>;
using job_not_available_atom = atom_constant<atom("job_na")>;
using job_ready_atom = atom_constant<atom("job_rdy")>;

behavior renderer(event_based_actor* self, const caf::actor &job_storage);
