/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator/object_bridge.h"

#include "util/v8_interact.hpp"

#include "native_generator.h"

object_bridge* object_bridge_ptr = nullptr;

object_bridge::object_bridge(int) {
  object_bridge_ptr = this;
}

void object_bridge::push_object(data_staging::circle& c) {
  circle.push_back(&c);
}
void object_bridge::pop_object() {
  circle.pop_back();
}

void object_bridge::set_generator(native_generator* ptr) {
  generator_ = ptr;
}

double object_bridge::get_x() const {
  return circle.back()->position_ref().x;
}
void object_bridge::set_x(double x) {
  circle.back()->position_ref().x = x;
}

double object_bridge::get_y() const {
  return y;
}
void object_bridge::set_y(double y) {
  this->y = y;
}
void object_bridge::spawn(v8::Local<v8::Object> obj) {
  data_staging::shape_t var = *circle.back();
  generator_->spawn_object(var, obj);
}

void object_bridge::add_to_context(v8pp::context& context) {
  v8pp::class_<object_bridge> object_bridge_class(context.isolate());
  object_bridge_class.ctor<int>()
      .set("x", v8pp::property(&object_bridge::get_x, &object_bridge::set_x))
      .set("y", v8pp::property(&object_bridge::get_y, &object_bridge::set_y))
      .set("spawn", &object_bridge::spawn);
  context.set("object_bridge", object_bridge_class);
}