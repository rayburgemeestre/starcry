#pragma once
#include "common.h"

using get_job_atom = atom_constant<atom("getjob")>;
using add_job_atom = atom_constant<atom("addjob")>;
using remove_job_atom = atom_constant<atom("removejob")>;
using create_jobs_atom = atom_constant<atom("createjobs")>;
using no_more_jobs_atom = atom_constant<atom("nomorejobs")>;
using num_jobs_atom = atom_constant<atom("numjobs")>;
using job_not_available_atom = atom_constant<atom("job_na")>;

behavior job_storage(event_based_actor* self);
