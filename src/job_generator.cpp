#include "job_generator.h"

size_t desired_num_jobs_queued = 10;

behavior job_generator_idle(event_based_actor* self, const caf::actor &job_storage) {
    return {
        [=](prepare_frame) {
            self->sync_send(job_storage, num_jobs_atom::value).then(
                [=](num_jobs_atom, size_t numjobs) {
                    if (numjobs < desired_num_jobs_queued) {
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
        [=](prepare_frame) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            self->sync_send(job_storage, add_job_atom::value, current_frame++, false).then(
                [=](no_more_jobs_atom) {
                    aout(self) << "job_generator: there are no more jobs to be generated." << endl;
                    self->become(nothing);
                },
                [=](num_jobs_atom, unsigned long numjobs) {
                    aout(self) << "job_generator: generated " << numjobs << " jobs now." << endl;
                    if (numjobs >= desired_num_jobs_queued) {
                        self->become(keep_behavior, job_generator_idle(self, job_storage));
                    }
                }
            );
        }
    };
}
