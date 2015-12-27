#include "job_generator.h"
#include "util/image_splitter.hpp"

// public
using start            = atom_constant<atom("start     ")>;

// external
using add_job          = atom_constant<atom("add_job   ")>;
using num_jobs         = atom_constant<atom("num_jobs  ")>;

// internal
using prepare_frame    = atom_constant<atom("prepare_fr")>;

size_t desired_num_jobs_queued = 10;

size_t current_job = 0;
size_t current_frame = 0;
size_t max_split_chunks = 0;
behavior job_generator(event_based_actor* self, const caf::actor &job_storage) {
    return {
        [=](start, size_t num_chunks) {
            max_split_chunks = num_chunks;
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
                    bool last_frame = (current_frame >= 10000);

                    ImageSplitter<uint32_t> imagesplitter{800, 600}; // fake values
                    const auto rectangles = imagesplitter.split(max_split_chunks);
                    size_t counter = 1;
                    for (const auto &rect : rectangles) {
                        self->send(job_storage, add_job::value, current_job++, current_frame, false, last_frame, counter, max_split_chunks);
                        counter++;
                    }
                    current_frame++;

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
