#include "job_generator.h"
#include "util/image_splitter.hpp"
#include "data/job.hpp"

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
                    bool last_frame = (current_frame >= 100);

                    // temporarily hardcoded dimensions
                    uint32_t width = 1280;
                    uint32_t height = 720;

                    ImageSplitter<uint32_t> imagesplitter{width, height}; // fake values
                    const auto rectangles = imagesplitter.split(max_split_chunks, ImageSplitter<uint32_t>::Mode::SplitHorizontal);
                    size_t counter = 1;

                    data::job new_job;
                    new_job.width = width;
                    new_job.height = height;
                    new_job.frame_number = current_frame;
                    new_job.rendered = false;
                    new_job.last_frame = last_frame;
                    new_job.num_chunks = max_split_chunks;

                    data::shape new_shape;
                    new_shape.x = 0;
                    new_shape.y = 0;
                    new_shape.z = 0;
                    new_shape.type = data::shape_type::circle;
                    new_shape.radius = 200.0 / 10000 * current_frame;
                    new_shape.radius_size = 1;
                    new_job.shapes.push_back(new_shape);

                    for (size_t i=0; i<rectangles.size(); i++) {
                        new_job.width = rectangles[i].width();
                        new_job.height = rectangles[i].height();
                        new_job.offset_x = rectangles[i].x();
                        new_job.offset_y = rectangles[i].y();
                        new_job.job_number = current_job++;
                        new_job.chunk = counter;
                        self->send(job_storage, add_job::value, new_job);
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
