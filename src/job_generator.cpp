#include "job_generator.h"
#include "util/image_splitter.hpp"
#include "data/job.hpp"

// public
using start            = atom_constant<atom("start     ")>;
using input_line       = atom_constant<atom("input_line")>;

// external
using add_job          = atom_constant<atom("add_job   ")>;
using num_jobs         = atom_constant<atom("num_jobs  ")>;

// internal
using prepare_frame    = atom_constant<atom("prepare_fr")>;

size_t desired_num_jobs_queued = 10;

size_t current_job = 0;
size_t current_frame = 0;
size_t max_split_chunks = 0;

#include "v8.h"
#include "libplatform/libplatform.h"
#include "v8pp/context.hpp"
#include "v8pp/function.hpp"

#include <memory>
#include <fstream>

using namespace std;

double get_version() { return 0.1; }
void output(string s) { cout << "received output: " << s << endl; }


class v8_wrapper {
public:
    v8_wrapper() : platform(nullptr), context(nullptr) {
        v8::V8::InitializeICU();
        platform = std::unique_ptr<v8::Platform>(v8::platform::CreateDefaultPlatform());
        v8::V8::InitializePlatform(platform.get());
        v8::V8::Initialize();
        context = new v8pp::context(); // Making it unique_ptr breaks it!
    }
    template<typename T = void>
    T run(std::string const& source)
    {
        v8::Isolate* isolate = context->isolate();
        v8::HandleScope scope(isolate);
        v8::TryCatch try_catch;
        v8::Handle<v8::Value> result = context->run_script(source);
        if (try_catch.HasCaught())
        {
            std::string const msg = v8pp::from_v8<std::string>(isolate, try_catch.Exception()->ToString());
            throw std::runtime_error(msg);
        }
        return v8pp::from_v8<T>(isolate, result);
    }
    template <typename T>
    inline void add_fun(const std::string &name, T func) {
        v8::HandleScope scope(context->isolate());
        context->set(name.c_str(), v8pp::wrap_function(context->isolate(), name.c_str(), func));
    }
    ~v8_wrapper() {
        delete(context);
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
    }
    v8pp::context * context;
    std::unique_ptr<v8::Platform> platform;
    std::mutex mut;
};

shared_ptr<v8_wrapper> context;

template<>
void v8_wrapper::run<void>(std::string const& source)
{
    v8::Isolate* isolate = context->isolate();
    v8::HandleScope scope(isolate);
    v8::TryCatch try_catch;
    v8::Handle<v8::Value> result = context->run_script(source);
    if (try_catch.HasCaught())
    {
        std::string const msg = v8pp::from_v8<std::string>(isolate, try_catch.Exception()->ToString());
        throw std::runtime_error(msg);
    }
}

behavior job_generator(event_based_actor *self, const caf::actor &job_storage, uint32_t canvas_w, uint32_t canvas_h, bool use_stdin) {

    context = make_shared<v8_wrapper>();

    //v8::HandleScope scope(ctx->context()->isolate());
    //auto global = v8::ObjectTemplate::New(ctx->context()->isolate());
    //v8::Handle<v8::Context> impl = v8::Context::New(ctx->context()->isolate(), nullptr, global);
    try {

        string filename = "test.js";
        ifstream stream(filename.c_str());
        if (!stream) {
            throw runtime_error("could not locate file " + filename);
        }
        istreambuf_iterator<char> begin(stream), end;

        context->add_fun("version", &get_version);
        context->run("var current_frame_ = 0;");
        context->run("var x = 0;");
        context->run("function current_frame() { return current_frame_; }");
        context->run(std::string(begin, end));
    }
    catch (exception & ex) {
        cout << ex.what() << endl;
    }

    return {
        [=](start, size_t num_chunks) {
            max_split_chunks = num_chunks;
            if (!use_stdin) {
                self->send(self, prepare_frame::value);
            }
        },
        [=](input_line, string line) {
            aout(self) << "input_line = " << context->run<string>("var x = x || 0; x++ ; 'test ' + x") << endl;
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
                    aout(self) << "job_generator: generated frame #" << current_frame << endl;

                    // Store it
                    bool last_frame = (current_frame >= 25 * 10);

                    uint32_t width = canvas_w;
                    uint32_t height = canvas_h;

                    ImageSplitter<uint32_t> imagesplitter{width, height}; // fake values
                    // We split horizontal always, because we simply concat the pixels later, so vertical would require a change in the code
                    const auto rectangles = imagesplitter.split(max_split_chunks, ImageSplitter<uint32_t>::Mode::SplitHorizontal);
                    size_t counter = 1;

                    //std::cout << "V8 reports version = " << context->run<double>("version()") << std::endl;
                    //std::cout << "V8 reports current_frame = " << context->run<double>("current_frame()") << std::endl;
                    double radius_test = context->run<double>("radius()");

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
                    context->run("current_frame_++;");

                    if (last_frame) {
                        aout(self) << "job_generator: there are no more jobs to be generated." << endl;
                        self->become(nothing);
                        //ctx.reset();
                        return;
                    }
                    self->send(self, prepare_frame::value);
                }
            );
        }
    };
}

