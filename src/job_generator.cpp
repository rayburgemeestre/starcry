#include "job_generator.h"

size_t desired_num_jobs_queued = 10;

size_t current_frame = 0;
behavior job_generator(event_based_actor* self, const caf::actor &job_storage) {
    return {
        [=](prepare_frame) {
            // create a frame
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            // send it to storage
            self->sync_send(job_storage, add_job_atom::value, current_frame++, false).then(
                [=](no_more_jobs_atom) {
                    aout(self) << "job_generator: there are no more jobs to be generated." << endl;
                    self->become(nothing);
                },
                [=](num_jobs_atom, unsigned long numjobs) {
                    if (numjobs >= desired_num_jobs_queued) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                    self->send(self, prepare_frame::value);
                }
            );
        }
    };
}
