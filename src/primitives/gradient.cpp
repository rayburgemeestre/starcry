/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "primitives.h"

using namespace std;

gradient::gradient() {}
#include <iostream>
const color gradient::get(double index) {
    size_t counter = 0;
    double processed_index = 0;
    for (const auto &pair : colors) {
        const double &current_idx = pair.first;
        if (current_idx > index) {
            double nom          = (index - processed_index);
            double denom        = (current_idx - processed_index);
            double color1_mult  = nom/denom;
            double color2_mult  = 1.0 - color1_mult;
            const color &color1 = colors[counter    ].second;
            const color &color2 = colors[counter - 1].second;
            return color( (color1.get_r() * color1_mult) + (color2.get_r() * color2_mult),
                          (color1.get_g() * color1_mult) + (color2.get_g() * color2_mult),
                          (color1.get_b() * color1_mult) + (color2.get_b() * color2_mult),
                          (color1.get_a() * color1_mult) + (color2.get_a() * color2_mult) );
        } else {
            processed_index = current_idx;
        }
        counter++;
    }
    color &c = colors[counter - 1].second;
    return color( c.get_r(), c.get_g(), c.get_b(), c.get_a() );
}
using namespace v8;
Local<Array> gradient::get2(double index) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    EscapableHandleScope handle_scope(isolate);
    Local<Array> array = Array::New(isolate, 4);
    if (array.IsEmpty())
        return Local<Array>();

    color c = get(index);
    array->Set(0, Number::New(isolate, c.get_r()));
    array->Set(1, Number::New(isolate, c.get_g()));
    array->Set(2, Number::New(isolate, c.get_b()));
    array->Set(3, Number::New(isolate, c.get_a()));
    return handle_scope.Escape(array);
}
using namespace v8;
#include <v8pp/class.hpp>
#include <v8pp/convert.hpp>
#include <sstream>
Local<Value> gradient::get3(double index) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    EscapableHandleScope scope(isolate);

    color c = get(index);
    std::stringstream ss;
    ss << "new color(" << c.get_r() << ", " << c.get_g() << ", " << c.get_b() << ", " << c.get_a() << ")";
	v8::Local<v8::Script> script = v8::Script::Compile(v8pp::to_v8(isolate, ss.str()), v8pp::to_v8(isolate, ""));
	v8::Local<v8::Value> result;
	if (!script.IsEmpty()) {
		result = script->Run();
	}
	return scope.Escape(result);
}

// temporary test
double gradient::get_r(double index) { return get(index).get_r(); }
double gradient::get_g(double index) { return get(index).get_g(); }
double gradient::get_b(double index) { return get(index).get_b(); }
double gradient::get_a(double index) { return get(index).get_a(); }
