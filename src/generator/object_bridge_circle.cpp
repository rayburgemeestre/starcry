/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator/object_bridge.h"

#include "util/v8_interact.hpp"

#include "native_generator.h"

object_bridge<data_staging::circle>* object_bridge_circle = nullptr;

template <>
object_bridge<data_staging::circle>::object_bridge(int) {
  object_bridge_circle = this;
}

template <>
double object_bridge<data_staging::circle>::get_x() const {
  return shape_stack.back()->location_ref().position_ref().x;
}

template <>
double object_bridge<data_staging::circle>::get_y() const {
  return shape_stack.back()->location_ref().position_ref().y;
}

template <>
double object_bridge<data_staging::circle>::get_z() const {
  return shape_stack.back()->location_ref().z();
}

template <>
double object_bridge<data_staging::circle>::get_radius() const {
  return shape_stack.back()->radius();
}

template <>
double object_bridge<data_staging::circle>::get_radius_size() const {
  return shape_stack.back()->radius_size();
}

template <>
void object_bridge<data_staging::circle>::set_x(double x) {
  shape_stack.back()->location_ref().position_ref().x = x;
}

template <>
void object_bridge<data_staging::circle>::set_y(double y) {
  shape_stack.back()->location_ref().position_ref().y = y;
}

template <>
void object_bridge<data_staging::circle>::set_z(double z) {
  shape_stack.back()->location_ref().set_z(z);
}

template <>
void object_bridge<data_staging::circle>::set_radius(double radius) {
  shape_stack.back()->set_radius(radius);
}

template <>
void object_bridge<data_staging::circle>::set_radius_size(double radius_size) {
  shape_stack.back()->set_radius_size(radius_size);
}

template <>
void object_bridge<data_staging::circle>::add_to_context(v8pp::context& context) {
  v8pp::class_<object_bridge> object_bridge_class(context.isolate());
  object_bridge_class.template ctor<int>()
      .set("x", v8pp::property(&object_bridge::get_x, &object_bridge::set_x))
      .set("y", v8pp::property(&object_bridge::get_y, &object_bridge::set_y))
      .set("z", v8pp::property(&object_bridge::get_z, &object_bridge::set_z))
      .set("radius", v8pp::property(&object_bridge::get_radius, &object_bridge::set_radius))
      .set("radiussize", v8pp::property(&object_bridge::get_radius_size, &object_bridge::set_radius_size))
      .set("spawn", &object_bridge::spawn);
  context.set("object_bridge_circle", object_bridge_class);
}

template class object_bridge<data_staging::circle>;