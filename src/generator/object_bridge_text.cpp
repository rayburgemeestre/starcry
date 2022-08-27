/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator.h"
#include "generator/object_bridge.h"
#include "util/v8_interact.hpp"

template <>
int64_t object_bridge<data_staging::text>::get_unique_id() const {
  return shape_stack.back()->meta_ref().unique_id();
}

template <>
double object_bridge<data_staging::text>::get_angle() const {
  return shape_stack.back()->generic_ref().angle();
}

template <>
double object_bridge<data_staging::text>::get_opacity() const {
  return shape_stack.back()->generic_ref().opacity();
}

template <>
double object_bridge<data_staging::text>::get_mass() const {
  return shape_stack.back()->generic_ref().mass();
}

template <>
double object_bridge<data_staging::text>::get_scale() const {
  return shape_stack.back()->generic_ref().scale();
}

template <>
double object_bridge<data_staging::text>::get_x() const {
  return shape_stack.back()->location_ref().position_ref().x;
}

template <>
double object_bridge<data_staging::text>::get_y() const {
  return shape_stack.back()->location_ref().position_ref().y;
}

template <>
double object_bridge<data_staging::text>::get_z() const {
  return shape_stack.back()->location_ref().z();
}

template <>
const std::string& object_bridge<data_staging::text>::get_text() const {
  return shape_stack.back()->text_cref();
}

template <>
double object_bridge<data_staging::text>::get_text_size() const {
  return shape_stack.back()->text_size();
}

template <>
const std::string& object_bridge<data_staging::text>::get_text_align() const {
  return shape_stack.back()->text_align();
}

template <>
bool object_bridge<data_staging::text>::get_text_fixed() const {
  return shape_stack.back()->text_fixed();
}

template <>
void object_bridge<data_staging::text>::set_unique_id(int64_t unique_id) {
  shape_stack.back()->meta_ref().set_unique_id(unique_id);
}

template <>
void object_bridge<data_staging::text>::set_angle(double angle) {
  shape_stack.back()->generic_ref().set_angle(angle);
}

template <>
void object_bridge<data_staging::text>::set_opacity(double opacity) {
  shape_stack.back()->generic_ref().set_opacity(opacity);
}

template <>
void object_bridge<data_staging::text>::set_mass(double mass) {
  shape_stack.back()->generic_ref().set_mass(mass);
}

template <>
void object_bridge<data_staging::text>::set_scale(double scale) {
  shape_stack.back()->generic_ref().set_scale(scale);
}
template <>
void object_bridge<data_staging::text>::set_x(double x) {
  shape_stack.back()->location_ref().position_ref().x = x;
}

template <>
void object_bridge<data_staging::text>::set_y(double y) {
  shape_stack.back()->location_ref().position_ref().y = y;
}

template <>
void object_bridge<data_staging::text>::set_z(double z) {
  shape_stack.back()->location_ref().set_z(z);
}

template <>
void object_bridge<data_staging::text>::set_text(const std::string& text) {
  shape_stack.back()->set_text(text);
}

template <>
void object_bridge<data_staging::text>::set_text_size(double text_size) {
  shape_stack.back()->set_text_size(text_size);
}

template <>
void object_bridge<data_staging::text>::set_text_align(const std::string& align) {
  shape_stack.back()->set_text_align(align);
}

template <>
void object_bridge<data_staging::text>::set_text_fixed(bool fixed) {
  shape_stack.back()->set_text_fixed(fixed);
}

template <>
double object_bridge<data_staging::text>::get_vel_x() const {
  return shape_stack.back()->movement_ref().velocity_ref().x;
}

template <>
double object_bridge<data_staging::text>::get_vel_y() const {
  return shape_stack.back()->movement_ref().velocity_ref().y;
}

template <>
double object_bridge<data_staging::text>::get_velocity() const {
  return shape_stack.back()->movement_ref().velocity_speed();
}

template <>
void object_bridge<data_staging::text>::set_vel_x(double x) {
  shape_stack.back()->movement_ref().velocity_ref().x = x;
}

template <>
void object_bridge<data_staging::text>::set_vel_y(double y) {
  shape_stack.back()->movement_ref().velocity_ref().y = y;
}

template <>
void object_bridge<data_staging::text>::set_velocity(double vel) {
  shape_stack.back()->movement_ref().set_velocity_speed(vel);
}

template <>
void object_bridge<data_staging::text>::set_seed(int64_t new_value) const {
  return shape_stack.back()->styling_ref().set_seed(new_value);
}

template <>
object_bridge<data_staging::text>::object_bridge(generator* generator) : generator_(generator) {
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
      .property("vel_x", &object_bridge::get_vel_x, &object_bridge::set_vel_x)
      .property("vel_y", &object_bridge::get_vel_y, &object_bridge::set_vel_y)
      .property("velocity", &object_bridge::get_velocity, &object_bridge::set_velocity)
      .property("text", &object_bridge::get_text, &object_bridge::set_text)
      .property("text_size", &object_bridge::get_text_size, &object_bridge::set_text_size)
      .property("text_align", &object_bridge::get_text_align, &object_bridge::set_text_align)
      .property("text_fixed", &object_bridge::get_text_fixed, &object_bridge::set_text_fixed)
      .property("props", &object_bridge::get_properties_local_ref)
      .property("gradients", &object_bridge::get_gradients_local_ref)
      .function("spawn", &object_bridge::spawn)
      .function("spawn3", &object_bridge::spawn3);
  instance_ = std::make_shared<v8::Persistent<v8::Object>>();
  (*instance_)
      .Reset(v8::Isolate::GetCurrent(), object_bridge_class.reference_external(v8::Isolate::GetCurrent(), this));
}

template class object_bridge<data_staging::text>;
