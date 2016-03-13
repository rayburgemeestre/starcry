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

#include "v8_wrapper.h"
#include "v8_wrapper_functions.hpp"

#include <memory>
#include <fstream>

double get_version() { return 0.1; };

std::shared_ptr<v8_wrapper> wrapper     = nullptr;
std::shared_ptr<v8_wrapper_context> ctx = nullptr;

void initialize_v8_wrapper() {
    wrapper = std::make_shared<v8_wrapper>();
    ctx = wrapper->context();
    try {
        std::string filename = "test.js";
        std::ifstream stream(filename.c_str());
        if (!stream) {
            throw std::runtime_error("could not locate file " + filename);
        }
        std::istreambuf_iterator<char> begin(stream), end;

        v8::Locker locker(ctx->context()->isolate());
        v8::HandleScope scope(ctx->context()->isolate());

        add_fun(ctx, "version", &get_version);
        ctx->run("var current_frame_ = 0;");
        ctx->run("function current_frame() { return current_frame_; }");
        ctx->run(std::string(begin, end));
    }
    catch (std::exception & ex) {
        std::cout << ex.what() << std::endl;
    }
}
void deinitialize_v8_wrapper() {
    ctx.reset();
    wrapper.reset();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
}


behavior job_generator(event_based_actor *self, const caf::actor &job_storage, uint32_t canvas_w, uint32_t canvas_h) {
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
                    bool last_frame = (current_frame >= 25 * 10);

                    uint32_t width = canvas_w;
                    uint32_t height = canvas_h;

                    ImageSplitter<uint32_t> imagesplitter{width, height}; // fake values
                    // We split horizontal always, because we simply concat the pixels later, so vertical would require a change in the code
                    const auto rectangles = imagesplitter.split(max_split_chunks, ImageSplitter<uint32_t>::Mode::SplitHorizontal);
                    size_t counter = 1;

                    v8::Locker locker(ctx->context()->isolate());
                    v8::HandleScope scope(ctx->context()->isolate());
                    //std::cout << "V8 reports version = " << ctx->run<double>("version()") << std::endl;
                    //std::cout << "V8 reports current_frame = " << ctx->run<double>("current_frame()") << std::endl;
                    double radius_test = ctx->run<double>("radius()");


                    data::job new_job;
                    new_job.width = width;
                    new_job.height = height;
                    new_job.frame_number = current_frame;
                    new_job.rendered = false;
                    new_job.last_frame = last_frame;
                    new_job.num_chunks = max_split_chunks;
                    new_job.canvas_w = width;
                    new_job.canvas_h = height;

                    data::shape new_shape;
                    new_shape.x = 0;
                    new_shape.y = 0;
                    new_shape.z = 0;
                    new_shape.type = data::shape_type::circle;
                    new_shape.radius = radius_test;
                    new_shape.radius_size = 5.0;
                    new_job.shapes.push_back(new_shape);

                    if (false)
                    for (int i=0; i<10; i++) {
                        data::shape new_shape;
                        new_shape.x = 0;
                        new_shape.y = 0;
                        new_shape.z = 0;
                        new_shape.type = data::shape_type::circle;
                        new_shape.radius = (current_frame + i*20) % 200;
                        new_shape.radius_size = 3.0;
                        new_job.shapes.push_back(new_shape);
                    }

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
                    ctx->run("current_frame_++;");

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
