/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator.h"
#include "interpreter/object_bridge.h"
#include "util/v8_interact.hpp"

template <>
int64_t object_bridge<data_staging::line>::get_unique_id() const {
  return shape_stack.back()->meta_ref().unique_id();
}

template <>
double object_bridge<data_staging::line>::get_angle() const {
  return shape_stack.back()->generic_ref().angle();
}

template <>
double object_bridge<data_staging::line>::get_opacity() const {
  return shape_stack.back()->generic_ref().opacity();
}

template <>
double object_bridge<data_staging::line>::get_mass() const {
  return shape_stack.back()->generic_ref().mass();
}

template <>
double object_bridge<data_staging::line>::get_scale() const {
  return shape_stack.back()->generic_ref().scale();
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
int64_t object_bridge<data_staging::line>::get_seed() const {
  return shape_stack.back()->styling_ref().seed();
}

template <>
void object_bridge<data_staging::line>::set_unique_id(int64_t unique_id) {
  shape_stack.back()->meta_ref().set_unique_id(unique_id);
}

template <>
void object_bridge<data_staging::line>::set_angle(double angle) {
  shape_stack.back()->generic_ref().set_angle(angle);
}

template <>
void object_bridge<data_staging::line>::set_opacity(double opacity) {
  shape_stack.back()->generic_ref().set_opacity(opacity);
}

template <>
void object_bridge<data_staging::line>::set_mass(double mass) {
  shape_stack.back()->generic_ref().set_mass(mass);
}

template <>
void object_bridge<data_staging::line>::set_scale(double scale) {
  shape_stack.back()->generic_ref().set_scale(scale);
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
void object_bridge<data_staging::line>::set_seed(int64_t new_value) const {
  return shape_stack.back()->styling_ref().set_seed(new_value);
}

template <>
object_bridge<data_staging::line>::object_bridge(interpreter::generator *generator) : generator_(generator) {
  v8pp::class_<object_bridge> object_bridge_class(v8::Isolate::GetCurrent());
  object_bridge_class  // .template ctor<int>()
      .property("level", &object_bridge::get_level)
      .property("unique_id", &object_bridge::get_unique_id, &object_bridge::set_unique_id)
      .property("angle", &object_bridge::get_angle, &object_bridge::set_angle)
      .property("opacity", &object_bridge::get_opacity, &object_bridge::set_opacity)
      .property("mass", &object_bridge::get_mass, &object_bridge::set_mass)
      .property("scale", &object_bridge::get_scale, &object_bridge::set_scale)
      .property("x", &object_bridge::get_x, &object_bridge::set_x)
      .property("y", &object_bridge::get_y, &object_bridge::set_y)
      .property("z", &object_bridge::get_z, &object_bridge::set_z)
      .property("x2", &object_bridge::get_x2, &object_bridge::set_x2)
      .property("y2", &object_bridge::get_y2, &object_bridge::set_y2)
      .property("z2", &object_bridge::get_z2, &object_bridge::set_z2)
      .property("radiussize", &object_bridge::get_radius_size, &object_bridge::set_radius_size)
      .property("spawn", &object_bridge::spawn)
      .property("spawn_parent", &object_bridge::spawn_parent)
      .property("props", &object_bridge::get_properties_local_ref)
      .property("gradients", &object_bridge::get_gradients_local_ref)
      .property("seed", &object_bridge::get_seed, &object_bridge::set_seed);
  instance_ = std::make_shared<v8::Persistent<v8::Object>>();
  (*instance_)
      .Reset(v8::Isolate::GetCurrent(), object_bridge_class.reference_external(v8::Isolate::GetCurrent(), this));
}

template class object_bridge<data_staging::line>;