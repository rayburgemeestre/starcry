#include "primitives.h"

#include "v8.h"
#include "libplatform/libplatform.h"
#include "v8pp/context.hpp"
#include "v8pp/function.hpp"
#include "v8pp/class.hpp"

void shape::add_to_context(v8pp::context &context)
{
    v8pp::class_<shape> shape_class(context.isolate());
    shape_class
        .ctor()
        .set("x", v8pp::property(&shape::get_x, &shape::set_x))
        .set("y", v8pp::property(&shape::get_y, &shape::set_y))
        .set("z", v8pp::property(&shape::get_z, &shape::set_z));
    context.set("shape", shape_class);
}

void circle::add_to_context(v8pp::context &context)
{
    v8pp::class_<circle> circle_class(context.isolate());
    circle_class
        .ctor<pos, double, double, gradient>() // TODO: try point or something
        .set("radius", v8pp::property(&circle::get_radius, &circle::set_radius))
        .inherit<shape>();
    context.set("circle", circle_class);
}

void line::add_to_context(v8pp::context &context)
{
    v8pp::class_<line> line_class(context.isolate());
    line_class
        .ctor<pos, pos, double, gradient>() // TODO: try point or something
        .set("x2", v8pp::property(&line::get_x2, &line::set_x2))
        .set("y2", v8pp::property(&line::get_y2, &line::set_y2))
        .set("z2", v8pp::property(&line::get_z2, &line::set_z2))
        .inherit<shape>();
    context.set("line", line_class);
}

void color::add_to_context(v8pp::context &context)
{
    v8pp::class_<color> color_class(context.isolate());
    color_class
        .ctor<double, double, double, double>()
        .set("r", v8pp::property(&color::get_r, &color::set_r))
        .set("g", v8pp::property(&color::get_g, &color::set_g))
        .set("b", v8pp::property(&color::get_b, &color::set_b))
        .set("a", v8pp::property(&color::get_a, &color::set_a));
    context.set("color", color_class);
}

void pos::add_to_context(v8pp::context &context)
{
    v8pp::class_<pos> pos_class(context.isolate());
    pos_class
        .ctor<double, double, double>()
        .set("x", v8pp::property(&pos::get_x, &pos::set_x))
        .set("y", v8pp::property(&pos::get_y, &pos::set_y))
        .set("z", v8pp::property(&pos::get_z, &pos::set_z));
    context.set("pos", pos_class);
}

void gradient::add_to_context(v8pp::context &context)
{
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
}
