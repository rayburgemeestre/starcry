#include "job_generator.h"

size_t desired_num_jobs_queued = 10;

size_t current_frame = 0;
behavior job_generator(event_based_actor* self, const caf::actor &job_storage) {
    return {
        [=](prepare_frame) {
            self->sync_send(job_storage, num_jobs_atom::value).then(
                [=](num_jobs_atom, unsigned long numjobs) {
                    // Renderer has some catching up to do
                    if (numjobs >= desired_num_jobs_queued) {
                        // retry in 200 milliseconds
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        self->send(self, prepare_frame::value);
                        return;
                    }

                    // Create a frame
                    aout(self) << "job_generator: generated frame #" << current_frame << endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));

                    // Store it
                    self->send(job_storage, add_job_atom::value, current_frame++, false);

                    // Our imaginary video is only 25 frames... so we are done
                    if (current_frame >= 25) {
                        aout(self) << "job_generator: there are no more jobs to be generated." << endl;
                        self->become(nothing);
                        return;
                    }
                    self->send(self, prepare_frame::value);
                }
            );
        }
    };
}
