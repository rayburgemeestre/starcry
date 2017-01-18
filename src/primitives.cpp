#include <v8pp/module.hpp>
#include "primitives.h"

#include "v8.h"
#include "libplatform/libplatform.h"
#include "v8pp/context.hpp"
#include "v8pp/function.hpp"
#include "v8pp/class.hpp"


v8::Local<v8::Value> circle::as_vec2d() {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
    std::string newvecfunc = "vector2d";
    v8::Handle<v8::Value> value = global->Get(v8::String::NewFromUtf8(isolate, newvecfunc.c_str(), v8::String::kNormalString, newvecfunc.length()));
	v8::Local<v8::Value> result;
    if (!value->IsFunction()) {
        return result;
    }
    v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(value);
    v8::Handle<v8::Value> args[2];
    args[0] = v8::Number::New(isolate, get_x());
    args[1] = v8::Number::New(isolate, get_y());
    return func->CallAsConstructor(isolate->GetCurrentContext(), 2, args).ToLocalChecked();
}

void shape::add_to_context(v8pp::context &context)
{
    v8pp::class_<shape> shape_class(context.isolate());
    shape_class
        .ctor()
        .set("x", v8pp::property(&shape::get_x, &shape::set_x))
        .set("y", v8pp::property(&shape::get_y, &shape::set_y))
        .set("z", v8pp::property(&shape::get_z, &shape::set_z))
        .set("blending_type", v8pp::property(&shape::get_blending_type, &shape::set_blending_type))
        ;
    context.set("shape", shape_class);

//    using data::blending_type;
//    v8pp::class_<data::blending_type> blending_type_enum_class(context.isolate());
//    blending_type_enum_class
//        .ctor<data::blending_type>();
//    context.set("blending_type", blending_type_enum_class);

    v8pp::module consts(context.isolate());
    consts
        .set_const("normal", data::blending_type::normal)
        .set_const("lighten", data::blending_type::lighten)
        .set_const("darken", data::blending_type::darken)
        .set_const("multiply", data::blending_type::multiply)
        .set_const("average", data::blending_type::average)
        .set_const("add", data::blending_type::add)
        .set_const("subtract", data::blending_type::subtract)
        .set_const("difference", data::blending_type::difference)
        .set_const("negation", data::blending_type::negation)
        .set_const("screen", data::blending_type::screen)
        .set_const("exclusion", data::blending_type::exclusion)
        .set_const("overlay", data::blending_type::overlay)
        .set_const("softlight", data::blending_type::softlight)
        .set_const("hardlight", data::blending_type::hardlight)
        .set_const("colordodge", data::blending_type::colordodge)
        .set_const("colorburn", data::blending_type::colorburn)
        .set_const("lineardodge", data::blending_type::lineardodge)
        .set_const("linearburn", data::blending_type::linearburn)
        .set_const("linearlight", data::blending_type::linearlight)
        .set_const("vividlight", data::blending_type::vividlight)
        .set_const("pinlight", data::blending_type::pinlight)
        .set_const("hardmix", data::blending_type::hardmix)
        .set_const("reflect", data::blending_type::reflect)
        .set_const("glow", data::blending_type::glow)
        .set_const("phoenix", data::blending_type::phoenix)
        .set_const("hue", data::blending_type::hue)
        .set_const("saturation", data::blending_type::saturation)
        .set_const("color", data::blending_type::color)
        .set_const("luminosity", data::blending_type::luminosity)
        ;
    context.set("blending_type", consts);
}

void circle::add_to_context(v8pp::context &context)
{
    v8pp::class_<circle> circle_class(context.isolate());
    circle_class
        .ctor<pos, double, double, gradient>() // TODO: try point or something
        .set("radius", v8pp::property(&circle::get_radius, &circle::set_radius))
        .set("radius_size", v8pp::property(&circle::get_radiussize, &circle::set_radiussize))
        .set("as_vec2d", &circle::as_vec2d)
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
        //.set("get", &gradient::get)
        //.set("get2", &gradient::get2)
        .set("get", &gradient::get3)
        .set("get_r", &gradient::get_r)
        .set("get_g", &gradient::get_g)
        .set("get_b", &gradient::get_b)
        .set("get_a", &gradient::get_a);
    context.set("gradient", gradient_class);
}
