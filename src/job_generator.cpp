/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "actors/job_generator.h"
#include "util/image_splitter.hpp"
#include "data/job.hpp"

// public
using start            = atom_constant<atom("start     ")>;
using input_line       = atom_constant<atom("input_line")>;
using no_more_input    = atom_constant<atom("no_more_in")>;
using debug            = atom_constant<atom("debug     ")>;
using show_stats       = atom_constant<atom("show_stats")>;

// external
using add_job          = atom_constant<atom("add_job   ")>;
using num_jobs         = atom_constant<atom("num_jobs  ")>;

// internal
using write_frame      = atom_constant<atom("write_fram")>;
using output_line      = atom_constant<atom("output_lin")>;
using next_frame       = atom_constant<atom("next_frame")>;

size_t desired_num_jobs_queued = 100;

size_t current_job = 0;
size_t current_frame = 0;
size_t max_split_chunks = 0;

#include "benchmark.h"
auto benchmark_class_ = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &fps_counter = static_cast<MeasureInterval &>(*benchmark_class_.get());

#include "v8.h"
#include "libplatform/libplatform.h"
#include "v8pp/context.hpp"
#include "v8pp/function.hpp"

#include <memory>
#include <fstream>

using namespace std;

double get_version() {
    return 0.1;
}
class v8_wrapper {
public:
    v8_wrapper(string filename) : context(nullptr), platform(nullptr), filename_(filename) {
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
        try_catch.SetVerbose(false);
        try_catch.SetCaptureMessage(true);
        v8::Handle<v8::Value> result = context->run_script(source);
        if (try_catch.HasCaught()) {
            rethrow_as_runtime_error(isolate, try_catch);
        }
        return v8pp::from_v8<T>(isolate, result);
    }

    // TODO: replace call(fn) and call(fn, T) with a template function
    void call(std::string const& function_name)
    {
        v8::Isolate* isolate = context->isolate();
        v8::HandleScope scope(isolate);
        v8::TryCatch try_catch;
        try_catch.SetVerbose(false);
        try_catch.SetCaptureMessage(true);

        v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
        v8::Local<v8::Function> func = global->Get(v8::String::NewFromUtf8(isolate, function_name.c_str())).As<v8::Function>();

        func->Call(global, 0, {});

        if (try_catch.HasCaught()) {
            rethrow_as_runtime_error(isolate, try_catch);
        }
        return; // v8pp::from_v8<T>(isolate, result);
    }

    template<typename T>
    void call(std::string const& function_name, T param)
    {
        v8::Isolate* isolate = context->isolate();
        v8::HandleScope scope(isolate);
        v8::TryCatch try_catch;
        try_catch.SetVerbose(false);
        try_catch.SetCaptureMessage(true);

        v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
        v8::Local<v8::Function> func = global->Get(v8::String::NewFromUtf8(isolate, function_name.c_str())).As<v8::Function>();

        v8::Handle<v8::Value> args[1];
        args[0] = v8pp::to_v8(isolate, param);
        func->Call(global, 1, args);

        if (try_catch.HasCaught()) {
            rethrow_as_runtime_error(isolate, try_catch);
        }
        return; // v8pp::from_v8<T>(isolate, result);
    }

    template <typename T>
    inline void add_fun(const std::string &name, T func) {
        v8::HandleScope scope(context->isolate());
        context->set(name.c_str(), v8pp::wrap_function(context->isolate(), name.c_str(), func));
    }
    template <typename T>
    inline void add_class(T func) {
        v8::HandleScope scope(context->isolate());
        func(*context);
    }
    ~v8_wrapper() {
        delete(context);
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
    }

    void rethrow_as_runtime_error(v8::Isolate *isolate, v8::TryCatch &try_catch) {
        auto ex = try_catch.Exception();
        std::string const msg = v8pp::from_v8<std::string>(isolate, ex->ToString());
        stringstream ss;
        v8::Handle<v8::Message> const & message( try_catch.Message() );
        if (!message.IsEmpty()) {
            int linenum = message->GetLineNumber();
            //cout << *v8::String::Utf8Value(message->GetScriptResourceName()) << ':'
            ss << "" << msg << '\n'
            << "[file: " << filename_ << ", line: " << std::dec << linenum << ".]\n"
            << *v8::String::Utf8Value(message->GetSourceLine()) << '\n';
            int start = message->GetStartColumn();
            int end   = message->GetEndColumn();
            for (int i = 0; i < start; i++) { ss << '-'; }
            for (int i = start; i < end; i++) { ss << '^'; }
            ss << '\n';
        }
        throw std::runtime_error(ss.str());
    }

    v8pp::context * context;
    std::unique_ptr<v8::Platform> platform;
    string filename_; // for error messages
    std::mutex mut;
};

shared_ptr<v8_wrapper> context;

template<>
void v8_wrapper::run<void>(std::string const& source)
{
    v8::Isolate* isolate = context->isolate();
    v8::HandleScope scope(isolate);
    v8::TryCatch try_catch;
    try_catch.SetVerbose(false);
    try_catch.SetCaptureMessage(true);
    context->run_script(source);
    if (try_catch.HasCaught()) {
        rethrow_as_runtime_error(isolate, try_catch);
    }
}

#include "primitives.h"

class assistent_ {
public:
    assistent_(event_based_actor *job_gen) : job_generator(job_gen) {}

    data::job the_job;
    event_based_actor *job_generator;
    size_t max_frames;
    bool realtime = false;
};
unique_ptr<assistent_> assistant = nullptr;

void add_text(double x, double y, double z, double textsize, string text, string align) {
    data::shape new_shape;
    new_shape.x = x;
    new_shape.y = y;
    new_shape.z = z;
    new_shape.text_size = textsize;
    new_shape.type = data::shape_type::text;
    new_shape.text = text;
    new_shape.align = align;
    assistant->the_job.shapes.push_back(new_shape);
}

void add_line(line l) {
    data::shape new_shape;
    new_shape.x = l.get_x();
    new_shape.y = l.get_y();
    new_shape.z = l.get_z();
    new_shape.x2 = l.get_x2();
    new_shape.y2 = l.get_y2();
    new_shape.z2 = l.get_z2();
/*  new_shape.r = r;
    new_shape.g = g;
    new_shape.b = b;  */
    new_shape.gradient_ = l.get_gradient().to_data_gradient();
    new_shape.type = data::shape_type::line;
    new_shape.radius_size = l.get_size();
    assistant->the_job.shapes.push_back(new_shape);
}

struct X
{
    int var = 1;

    int get() const { return var; }
    void set(int v) { var = v; }

    int fun(int x) { return var + x; }
    static int static_fun(int x) { return x; }
};

struct Y : X
{
    static int instance_count;

    explicit Y(double x) { var = x; ++instance_count; }
    ~Y() { --instance_count; }
};

int Y::instance_count = 0;

void set_background_color(color clr) {
    assistant->the_job.background_color.r = clr.get_r();
    assistant->the_job.background_color.g = clr.get_g();
    assistant->the_job.background_color.b = clr.get_b();
    assistant->the_job.background_color.a = clr.get_a();
}

void add_circle(circle circ) {
    data::shape new_shape;
    new_shape.x = circ.get_x();
    new_shape.y = circ.get_y();
    new_shape.z = circ.get_z();
    new_shape.type = data::shape_type::circle;
    new_shape.radius = circ.get_radius();
    new_shape.radius_size = circ.get_radiussize();
    new_shape.gradient_ = circ.get_gradient().to_data_gradient();
    assistant->the_job.shapes.push_back(new_shape);
}


void output(string s) {
    assistant->job_generator->send(assistant->job_generator, output_line::value, s);
}

void write_frame_fun_impl(bool last_frame) {
    if (!assistant->the_job.last_frame)
        assistant->the_job.last_frame = last_frame || (assistant->max_frames && assistant->max_frames == current_frame);
    assistant->job_generator->send(assistant->job_generator, write_frame::value, assistant->the_job);
    assistant->the_job.shapes.clear();
}
void close_fun() {
    write_frame_fun_impl(true);
}
void write_frame_fun() {
    write_frame_fun_impl(false);
}

#include "v8pp/class.hpp"

void call_print_exception(event_based_actor *self, string fn)
{
    try {
        context->call(fn);
    }
    catch (std::runtime_error &ex) {
        aout(self) << "Runtime error caused during execution of "<< fn << "() in javascript:" << endl << string(ex.what()) << endl;
        self->quit(exit_reason::user_shutdown);
    }
}

template <typename T>
void call_print_exception(event_based_actor *self, string fn, T arg)
{
    try {
        context->call(fn, arg);
    }
    catch (std::runtime_error &ex) {
        aout(self) << "Runtime error caused during execution of "<< fn << "(..) in javascript:" << endl << string(ex.what()) << endl;
        self->quit(exit_reason::user_shutdown);
    }
}

behavior job_generator(event_based_actor *self, const caf::actor &job_storage, const string &filename, uint32_t canvas_w, uint32_t canvas_h, bool use_stdin, bool rendering_enabled) {
    self->link_to(job_storage);
    fps_counter.setDescription("fps");
    fps_counter.startHistogramAtZero(true);
    context = make_shared<v8_wrapper>(filename);
    try {
        ifstream stream(filename.c_str());
        if (!stream) {
            throw runtime_error("could not locate file " + filename);
        }

        context->add_class([](auto &context){

            // add shape class
            v8pp::class_<shape> shape_class(context.isolate());
            shape_class
                .ctor()
                .set("x", v8pp::property(&shape::get_x, &shape::set_x))
                .set("y", v8pp::property(&shape::get_y, &shape::set_y))
                .set("z", v8pp::property(&shape::get_z, &shape::set_z));
            context.set("shape", shape_class);

            // add circle class
            v8pp::class_<circle> circle_class(context.isolate());
            circle_class
                .ctor<pos, double, double, gradient>() // TODO: try point or something
                .set("radius", v8pp::property(&circle::get_radius, &circle::set_radius))
                .inherit<shape>();
            context.set("circle", circle_class);

            // add line class
            v8pp::class_<line> line_class(context.isolate());
            line_class
                .ctor<pos, pos, double, gradient>() // TODO: try point or something
                .set("x2", v8pp::property(&line::get_x2, &line::set_x2))
                .set("y2", v8pp::property(&line::get_y2, &line::set_y2))
                .set("z2", v8pp::property(&line::get_z2, &line::set_z2))
                .inherit<shape>();
            context.set("line", line_class);

            // add color class
            v8pp::class_<color> color_class(context.isolate());
            color_class
                .ctor<double, double, double, double>()
                .set("r", v8pp::property(&color::get_r, &color::set_r))
                .set("g", v8pp::property(&color::get_g, &color::set_g))
                .set("b", v8pp::property(&color::get_b, &color::set_b))
                .set("a", v8pp::property(&color::get_a, &color::set_a));
            context.set("color", color_class);

            // add pos class
            v8pp::class_<pos> pos_class(context.isolate());
            pos_class
                .ctor<double, double, double>()
                .set("x", v8pp::property(&pos::get_x, &pos::set_x))
                .set("y", v8pp::property(&pos::get_y, &pos::set_y))
                .set("z", v8pp::property(&pos::get_z, &pos::set_z));
            context.set("pos", pos_class);

            // add gradient class
            v8pp::class_<gradient> gradient_class(context.isolate());
            gradient_class
                .ctor<>()
                .set("add", &gradient::add_color)
                .set("get", &gradient::get)
                .set("get2", &gradient::get2)
                .set("get3", &gradient::get3)
                .set("get_r", &gradient::get_r)
                .set("get_g", &gradient::get_g)
                .set("get_b", &gradient::get_b)
                .set("get_a", &gradient::get_a);
            context.set("gradient", gradient_class);

            v8pp::class_<X> X_class(context.isolate());
            X_class
                .ctor()
                .set_const("konst", 99)
                .set("var", &X::var)
                .set("rprop", v8pp::property(&X::get))
                .set("wprop", v8pp::property(&X::get, &X::set))
                .set("fun", &X::fun)
                .set("static_fun", &X::static_fun)
                .set("static_lambda", [](int x) { return x + 3; })
                ;

            v8pp::class_<Y> Y_class(context.isolate());
            Y_class
                .inherit<X>()
                .ctor<double>()
                ;

            context
                .set("X", X_class)
                .set("Y", Y_class)
                ;
        });


        context->add_fun("version", &get_version);
        context->add_fun("output", &output);
        context->add_fun("write_frame", &write_frame_fun);
        context->add_fun("close", &close_fun);
        context->add_fun("add_text", &add_text);
        context->add_fun("set_background_color", &set_background_color);
        context->add_fun("add_circle", &add_circle);
        context->add_fun("add_line", &add_line);
        context->run("var current_frame = 0;");
        istreambuf_iterator<char> begin(stream), end;

        context->run(std::string(begin, end));

        canvas_w = context->run<uint32_t>(string("typeof canvas_w != 'undefined' ? canvas_w : ") + to_string(canvas_w));
        canvas_h = context->run<uint32_t>(string("typeof canvas_h != 'undefined' ? canvas_h : ") + to_string(canvas_h));

        if (context->run<bool>("typeof initialize != 'undefined'")) {
            call_print_exception(self, "initialize");
        }
    }
    catch (exception & ex) {
        cout << ex.what() << endl;
    }
    return {
        [=](start, size_t num_chunks) {
            max_split_chunks = num_chunks;
            assistant = std::make_unique<assistent_>(self);
            assistant->the_job.width         = canvas_w;
            assistant->the_job.height        = canvas_h;
            assistant->the_job.frame_number  = current_frame;
            assistant->the_job.rendered      = false;
            assistant->the_job.last_frame    = false;
            assistant->the_job.num_chunks    = max_split_chunks;
            assistant->the_job.canvas_w      = canvas_w;
            assistant->the_job.canvas_h      = canvas_h;
            assistant->the_job.scale         = context->run<double>("typeof scale != 'undefined' ? scale : 1.0");
            assistant->the_job.bitrate       = context->run<double>("typeof bitrate != 'undefined' ? bitrate : (500 * 1024 * 8)");
            assistant->max_frames            = context->run<size_t>("typeof max_frames != 'undefined' ? max_frames : 250");
            assistant->realtime              = context->run<bool>("typeof realtime != 'undefined' ? realtime : false");
            if (!use_stdin || assistant->realtime) {
                // we call into V8 ourselves to get frames
                call_print_exception(self, "next");
            }
        },
        [=](input_line, string line) {
            call_print_exception(self, "input", line);
        },
        [=](no_more_input) {
            assistant->the_job.last_frame = true;
            call_print_exception(self, "close");
        },
        [=](write_frame, data::job &job) {
            /*
             There is something wrong with this logic
             ----------------------------------------
             Edit: maybe because of the reference?
             Edit2: No, you just cannot put it here, at some point its processing the last job for instance...
                    put the delay / wait somewhere else..
             
            // Every 1000th frame see if we're not flooding the job storage,
            //  if so, pause processing.
            static bool generator_paused = false;
            if (generator_paused) {
                self->request(job_storage, infinite, num_jobs::value).await(
                    [=](num_jobs, unsigned long numjobs) {
                        // Unpause if "queue" is half empty
                        if (numjobs <= (desired_num_jobs_queued / 2.0)) {
                            generator_paused = false;
                        }
                    }
                );
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                self->send(self, write_frame::value, job);
                return;
            }
            if (job.frame_number % 1000 == 0) {
                self->request(job_storage, infinite, num_jobs::value).await(
                    [=](num_jobs, unsigned long numjobs) {
                        // Too much generated.. pause job generator
                        if (numjobs >= desired_num_jobs_queued) {
                            generator_paused = true;
                            // let's try again in 100 milliseconds..
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            self->send(self, write_frame::value, job);
                            return;
                        }
                    }
                );
            }
            */

            uint32_t width = canvas_w;
            uint32_t height = canvas_h;

            ImageSplitter<uint32_t> imagesplitter{width, height}; // fake values
            // We split horizontal always, because we simply concat the pixels later, so vertical would require a change in the code
            const auto rectangles = imagesplitter.split(max_split_chunks, ImageSplitter<uint32_t>::Mode::SplitHorizontal);
            size_t counter = 1;

            //std::cout << "V8 reports version = " << context->run<double>("version()") << std::endl;
            //std::cout << "V8 reports current_frame = " << context->run<double>("current_frame()") << std::endl;

            job.frame_number = current_frame;

            for (size_t i=0; i<rectangles.size(); i++) {
                job.width = rectangles[i].width();
                job.height = rectangles[i].height();
                job.offset_x = rectangles[i].x();
                job.offset_y = rectangles[i].y();
                job.job_number = current_job++;
                job.chunk = counter;
                if (rendering_enabled) {
                    self->send(job_storage, add_job::value, job);
                }
                counter++;
            }
            current_frame++;
            // in some cases a javascript may want manual control over incrementing the current frame.
            // i.e., if you buffer multiple lines, and after X time manually output a frame..
            //if (!assistant->control_current_frame) {
            context->run("current_frame = " + to_string(current_frame) + ";");

            //std::cout << "RBU7: created job frame number " << job.frame_number << endl;
           // }
            fps_counter.measure();
            job.shapes.clear();
            if (job.last_frame) {
                aout(self) << "job_generator: there are no more jobs to be generated." << endl;
                // self->become(nothing); don't do this, v8 might be calling into us still..
                if (!rendering_enabled) {
                    // this actor shuts down the system in that case
                    self->quit(exit_reason::user_shutdown);
                }
                return;
            }

            if (!use_stdin || assistant->realtime) {
                self->send(self, next_frame::value);
            }
        },
        [=](output_line, string line) {
            aout(self) << line << endl;
        },
        [=](next_frame) {
            self->request(job_storage, infinite, num_jobs::value).then(
                [=](num_jobs, unsigned long numjobs) {
                    // Renderer has some catching up to do
                    if (numjobs >= desired_num_jobs_queued) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        self->send(self, next_frame::value);
                        return;
                    }
                    call_print_exception(self, "next");
                }
            );
        },
        [=](debug) {
            aout(self) << "job_generator mailbox = " << self->mailbox().count() << " " << self->mailbox().counter() << endl;
        },
        [=](show_stats) {
            aout(self) << "generator at frame: " << current_frame << ", with frames/sec: " << (1000.0 / fps_counter.mean())
                       << " +/- " << fps_counter.stderr() << " mailbox: " << self->mailbox().count() << endl;
        },
        [=](num_jobs) -> message {
            return make_message(num_jobs::value, self->mailbox().count());
        },
    };
}

