/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator/object_bridge.h"

#include "util/v8_interact.hpp"

#include "native_generator.h"

object_bridge<data_staging::line>* object_bridge_line = nullptr;

template <>
object_bridge<data_staging::line>::object_bridge(int) {
  object_bridge_line = this;
}

template <>
double object_bridge<data_staging::line>::get_x() const {
  return shape_stack.back()->line_start_ref().position_ref().x;
}

template <>
double object_bridge<data_staging::line>::get_y() const {
  return shape_stack.back()->line_start_ref().position_ref().y;
}

template <>
double object_bridge<data_staging::line>::get_z() const {
  return shape_stack.back()->line_start_ref().z();
}

template <>
double object_bridge<data_staging::line>::get_x2() const {
  return shape_stack.back()->line_end_ref().position_ref().x;
}

template <>
double object_bridge<data_staging::line>::get_y2() const {
  return shape_stack.back()->line_end_ref().position_ref().y;
}

template <>
double object_bridge<data_staging::line>::get_z2() const {
  return shape_stack.back()->line_end_ref().z();
}

template <>
double object_bridge<data_staging::line>::get_radius_size() const {
  return shape_stack.back()->line_width();
}

template <>
void object_bridge<data_staging::line>::set_x(double x) {
  shape_stack.back()->line_start_ref().position_ref().x = x;
}

template <>
void object_bridge<data_staging::line>::set_y(double y) {
  shape_stack.back()->line_start_ref().position_ref().y = y;
}

template <>
void object_bridge<data_staging::line>::set_z(double z) {
  shape_stack.back()->line_start_ref().set_z(z);
}

template <>
void object_bridge<data_staging::line>::set_x2(double x) {
  shape_stack.back()->line_end_ref().position_ref().x = x;
}

template <>
void object_bridge<data_staging::line>::set_y2(double y) {
  shape_stack.back()->line_end_ref().position_ref().y = y;
}

template <>
void object_bridge<data_staging::line>::set_z2(double z) {
  shape_stack.back()->line_end_ref().set_z(z);
}

template <>
void object_bridge<data_staging::line>::set_radius_size(double line_width) {
  shape_stack.back()->set_line_width(line_width);
}

template <>
void object_bridge<data_staging::line>::add_to_context(v8pp::context& context) {
  v8pp::class_<object_bridge> object_bridge_class(context.isolate());
  object_bridge_class.template ctor<int>()
      .set("x", v8pp::property(&object_bridge::get_x, &object_bridge::set_x))
      .set("y", v8pp::property(&object_bridge::get_y, &object_bridge::set_y))
      .set("z", v8pp::property(&object_bridge::get_z, &object_bridge::set_z))
      .set("x2", v8pp::property(&object_bridge::get_x2, &object_bridge::set_x2))
      .set("y2", v8pp::property(&object_bridge::get_y2, &object_bridge::set_y2))
      .set("z2", v8pp::property(&object_bridge::get_z2, &object_bridge::set_z2))
      .set("radiussize", v8pp::property(&object_bridge::get_radius_size, &object_bridge::set_radius_size))
      .set("spawn", &object_bridge::spawn);
  context.set("object_bridge_line", object_bridge_class);
}

template class object_bridge<data_staging::line>;