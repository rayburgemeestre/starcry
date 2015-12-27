#include "job_generator.h"

// public
using start            = atom_constant<atom("start     ")>;

// external
using add_job          = atom_constant<atom("add_job   ")>;
using num_jobs         = atom_constant<atom("num_jobs  ")>;

// internal
using prepare_frame    = atom_constant<atom("prepare_fr")>;

size_t desired_num_jobs_queued = 10;

size_t current_frame = 0;
behavior job_generator(event_based_actor* self, const caf::actor &job_storage) {
    return {
        [=](start) {
            self->send(self, prepare_frame::value);
        },
        [=](prepare_frame) {
            self->sync_send(job_storage, num_jobs::value).then(
                [=](num_jobs, unsigned long numjobs) {
                    // Renderer has some catching up to do
                    if (numjobs >= desired_num_jobs_queued) {
                        self->send(self, prepare_frame::value);
                        return;
                    }

                    // Create a frame
                    //aout(self) << "job_generator: generated frame #" << current_frame << endl;

                    // Store it
                    bool last_frame = (current_frame >= 100000);
                    self->send(job_storage, add_job::value, current_frame++, false, last_frame);

                    if (last_frame) {
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
