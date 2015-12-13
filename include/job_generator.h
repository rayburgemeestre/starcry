#pragma once

#include "common.h"

using add_job_atom = atom_constant<atom("addjob")>;
using no_more_jobs_atom = atom_constant<atom("nomorejobs")>;
using num_jobs_atom = atom_constant<atom("numjobs")>;
using create_jobs_atom = atom_constant<atom("createjobs")>;

behavior job_generator_idle(event_based_actor* self, const caf::actor &job_storage);
behavior job_generator(event_based_actor* self, const caf::actor &job_storage);
