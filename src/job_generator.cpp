#include "job_generator.h"

behavior job_generator_idle(event_based_actor* self, const caf::actor &job_storage) {
	return {
		[=](create_jobs_atom) {
			self->sync_send(job_storage, num_jobs_atom::value).then(
				[=](num_jobs_atom, size_t numjobs) {
					if (numjobs < 10) {
						self->unbecome();
					}
				}
			);
		}
	};
}


size_t current_frame = 0;
behavior job_generator(event_based_actor* self, const caf::actor &job_storage) {
	return {
		[=](create_jobs_atom) {
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			self->sync_send(job_storage, add_job_atom::value, current_frame++, false).then(
				[=](no_more_jobs_atom) {
					aout(self) << "job_generator: there are no more jobs to be generated." << endl;
					self->become(nothing);
				},
				[=](num_jobs_atom, unsigned long numjobs) {
					aout(self) << "job_generator: generated " << numjobs << " jobs now." << endl;
					if (numjobs >= 10) {
						self->become(keep_behavior, job_generator_idle(self, job_storage));
					}
				}
			);
		}
	};
}
