#include "actors/job_generator.h"
#include "util/image_splitter.hpp"
#include "data/job.hpp"

// public
using start            = atom_constant<atom("start     ")>;
using input_line       = atom_constant<atom("input_line")>;
using no_more_input    = atom_constant<atom("no_more_in")>;

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
    template<typename T>
    void call(std::string const& function_name, T param)
    {
        v8::Isolate* isolate = context->isolate();
        v8::HandleScope scope(isolate);
        v8::TryCatch try_catch;

        v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
        v8::Local<v8::Function> func = global->Get(v8::String::NewFromUtf8(isolate, function_name.c_str())).As<v8::Function>();

        v8::Handle<v8::Value> args[1];
        args[0] = v8pp::to_v8(isolate, param);
        v8::Local<v8::Value> result = func->Call(global, 1, args);

        if (try_catch.HasCaught()) {
            std::string const msg = v8pp::from_v8<std::string>(isolate, try_catch.Exception()->ToString());
            throw std::runtime_error(msg);
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

class assistent_ {
public:
    assistent_(event_based_actor *job_gen) : job_generator(job_gen) {}

    data::job the_job;
    event_based_actor *job_generator;
    size_t max_frames;
    bool realtime = false;
};
unique_ptr<assistent_> assistant = nullptr;

void add_text(double x, double y, double z, string text, string align) {
    data::shape new_shape;
    new_shape.x = x;
    new_shape.y = y;
    new_shape.z = z;
    new_shape.type = data::shape_type::text;
    new_shape.text = text;
    new_shape.align = align;
    assistant->the_job.shapes.push_back(new_shape);
}
void add_circle(double x, double y, double z, double radius, double radius_size) {
    data::shape new_shape;
    new_shape.x = x;
    new_shape.y = y;
    new_shape.z = z;
    new_shape.type = data::shape_type::circle;
    new_shape.radius = radius;
    new_shape.radius_size = radius_size;
    assistant->the_job.shapes.push_back(new_shape);
}
void add_line(double x, double y, double z, double x2, double y2, double z2, double size,
              double r, double g, double b) {
    data::shape new_shape;
    new_shape.x = x;
    new_shape.y = y;
    new_shape.z = z;
    new_shape.x2 = x2;
    new_shape.y2 = y2;
    new_shape.z2 = z2;
    new_shape.r = r;
    new_shape.g = g;
    new_shape.b = b;
    new_shape.type = data::shape_type::line;
    new_shape.radius_size = size;
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

struct pos
{
    double x_;
    double y_;
    double z_;

    explicit pos(double x, double y, double z) {
        x_ = x;
        y_ = y;
        z_ = z;
    }
    double get_x() const { return x_; }
    double get_y() const { return y_; }
    double get_z() const { return z_; }
    void set_x(double x) { x_ = x; }
    void set_y(double y) { y_ = y; }
    void set_z(double z) { z_ = z; }
};

struct shape
{
    double x_;
    double y_;
    double z_;

    double get_x() const { return x_; }
    double get_y() const { return y_; }
    double get_z() const { return z_; }
    void set_x(double x) { x_ = x; }
    void set_y(double y) { y_ = y; }
    void set_z(double z) { z_ = z; }
};

struct line : shape
{
    double x2_;
    double y2_;
    double z2_;

    explicit line(double x, double y, double z, double x2, double y2, double z2) {
        set_x(x);
        set_y(y);
        set_z(z);
        set_x2(x2);
        set_y2(y2);
        set_z2(z2);
    }

    double get_x2() const { return x2_; }
    double get_y2() const { return y2_; }
    double get_z2() const { return z2_; }
    void set_x2(double x) { x2_ = x; }
    void set_y2(double y) { y2_ = y; }
    void set_z2(double z) { z2_ = z; }
};

struct color
{
    double r_;
    double g_;
    double b_;
    double a_;

    explicit color(double r, double g, double b, double a) {
        set_r(r);
        set_g(g);
        set_b(b);
        set_a(a);
    }

    double get_r() const { return r_; }
    double get_g() const { return g_; }
    double get_b() const { return b_; }
    double get_a() const { return a_; }
    void set_r(double r) { r_ = r; }
    void set_g(double g) { g_ = g; }
    void set_b(double b) { b_ = b; }
    void set_a(double a) { a_ = a; }
};

struct circle : shape
{
    double radius_;
    double radiussize_;
    color color_;

    explicit circle(pos p, double radius, double radiussize, color c)
        : color_(c)
    {
        set_x(p.get_x());
        set_y(p.get_y());
        set_z(p.get_z());
        set_radius(radius);
        set_radiussize(radiussize);
    }

    double get_radius() const { return radius_; }
    void set_radius(double r) { radius_ = r; }
    double get_radiussize() const { return radiussize_; }
    void set_radiussize(double r) { radiussize_ = r; }
    color get_color() const { return color_; }
    void set_color(color c) { color_ = c; }
};


int Y::instance_count = 0;

// test

void add_circle2(circle circ) {
    data::shape new_shape;
    new_shape.x = circ.get_x();
    new_shape.y = circ.get_y();
    new_shape.z = circ.get_z();
    new_shape.type = data::shape_type::circle;
    new_shape.radius = circ.get_radius();
    new_shape.radius_size = circ.get_radiussize();
    new_shape.r = circ.get_color().get_r();
    new_shape.g = circ.get_color().get_g();
    new_shape.b = circ.get_color().get_b();
    assistant->the_job.shapes.push_back(new_shape);
}


void output(string s) {
    assistant->job_generator->send(assistant->job_generator, output_line::value, s);
}

void write_frame_fun1(bool last_frame) {
    if (!assistant->the_job.last_frame)
        assistant->the_job.last_frame = last_frame || assistant->max_frames == current_frame;
    assistant->job_generator->send(assistant->job_generator, write_frame::value, assistant->the_job);
    assistant->the_job.shapes.clear();
}
void write_frame_fun() {
    write_frame_fun1(false);
}

#include "v8pp/class.hpp"

behavior job_generator(event_based_actor *self, const caf::actor &job_storage, const string &filename, uint32_t canvas_w, uint32_t canvas_h, bool use_stdin) {
    context = make_shared<v8_wrapper>();
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
                .ctor<pos, double, double, color>() // TODO: try point or something
                .set("radius", v8pp::property(&circle::get_radius, &circle::set_radius))
                .inherit<shape>();
            context.set("circle", circle_class);

            // add line class
            v8pp::class_<line> line_class(context.isolate());
            line_class
                .ctor<double, double, double, double, double, double>() // TODO: try point or something
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
        context->add_fun("write_frame1", &write_frame_fun1);
        context->add_fun("add_text", &add_text);
        context->add_fun("add_circle", &add_circle);
        context->add_fun("add_circle2", &add_circle2);
        context->add_fun("add_line", &add_line);
        context->run("var current_frame = 0;");
        context->run("var x = 0;");
        istreambuf_iterator<char> begin(stream), end;
        context->run(std::string(begin, end));

        canvas_w = context->run<uint32_t>(string("typeof canvas_w != 'undefined' ? canvas_w : ") + to_string(canvas_w));
        canvas_h = context->run<uint32_t>(string("typeof canvas_h != 'undefined' ? canvas_h : ") + to_string(canvas_h));

        context->call("initialize", "");
    }
    catch (exception & ex) {
        cout << ex.what() << endl;
    }
    return {
        [=](start, size_t num_chunks) {
            max_split_chunks = num_chunks;
            assistant = std::make_unique<assistent_>(self);
            assistant->the_job.width        = canvas_w;
            assistant->the_job.height       = canvas_h;
            assistant->the_job.frame_number = current_frame;
            assistant->the_job.rendered     = false;
            assistant->the_job.last_frame   = false;
            assistant->the_job.num_chunks   = max_split_chunks;
            assistant->the_job.canvas_w     = canvas_w;
            assistant->the_job.canvas_h     = canvas_h;
            assistant->the_job.scale        = context->run<double>("scale || 1.0");
            assistant->max_frames           = context->run<size_t>("max_frames");
            assistant->realtime             = context->run<bool>("realtime");
            if (!use_stdin || assistant->realtime) {
                // we call into V8 ourselves to get frames
                context->call("next", "");
            }
        },
        [=](input_line, string line) {
            context->call("input", line);
        },
        [=](no_more_input) {
            assistant->the_job.last_frame = true;
            context->call("close", "");
        },
        [=](write_frame, data::job &job) {
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
                self->send(job_storage, add_job::value, job);
                counter++;
            }
            current_frame++;
            context->run("current_frame++;");
            if (job.last_frame) {
                aout(self) << "job_generator: there are no more jobs to be generated." << endl;
                self->become(nothing);
                return;
            }
            job.shapes.clear();

            if (!use_stdin || assistant->realtime) {
                self->send(self, next_frame::value);
            }
        },
        [=](output_line, string line) {
            aout(self) << line << endl;
        },
        [=](next_frame) {
            self->sync_send(job_storage, num_jobs::value).then(
                [=](num_jobs, unsigned long numjobs) {
                    // Renderer has some catching up to do
                    if (numjobs >= desired_num_jobs_queued) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        self->send(self, next_frame::value);
                        return;
                    }
                    context->call("next", "");
                }
            );
        }
    };
}

