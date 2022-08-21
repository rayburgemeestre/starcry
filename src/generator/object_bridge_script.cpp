/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator.h"
#include "generator/object_bridge.h"
#include "util/v8_interact.hpp"

template <>
int64_t object_bridge<data_staging::script>::get_unique_id() const {
  return shape_stack.back()->meta_ref().unique_id();
}

template <>
double object_bridge<data_staging::script>::get_angle() const {
  return shape_stack.back()->generic_ref().angle();
}

template <>
double object_bridge<data_staging::script>::get_opacity() const {
  return shape_stack.back()->generic_ref().opacity();
}

template <>
double object_bridge<data_staging::script>::get_mass() const {
  return shape_stack.back()->generic_ref().mass();
}

template <>
double object_bridge<data_staging::script>::get_scale() const {
  return shape_stack.back()->generic_ref().scale();
}

template <>
double object_bridge<data_staging::script>::get_x() const {
  return shape_stack.back()->location_ref().position_ref().x;
}

template <>
double object_bridge<data_staging::script>::get_y() const {
  return shape_stack.back()->location_ref().position_ref().y;
}

template <>
double object_bridge<data_staging::script>::get_z() const {
  return shape_stack.back()->location_ref().z();
}

template <>
void object_bridge<data_staging::script>::set_unique_id(int64_t unique_id) {
  shape_stack.back()->meta_ref().set_unique_id(unique_id);
}

template <>
void object_bridge<data_staging::script>::set_angle(double angle) {
  shape_stack.back()->generic_ref().set_angle(angle);
}

template <>
void object_bridge<data_staging::script>::set_opacity(double opacity) {
  shape_stack.back()->generic_ref().set_opacity(opacity);
}

template <>
void object_bridge<data_staging::script>::set_mass(double mass) {
  shape_stack.back()->generic_ref().set_mass(mass);
}

template <>
void object_bridge<data_staging::script>::set_scale(double scale) {
  shape_stack.back()->generic_ref().set_scale(scale);
}
template <>
void object_bridge<data_staging::script>::set_x(double x) {
  shape_stack.back()->location_ref().position_ref().x = x;
}

template <>
void object_bridge<data_staging::script>::set_y(double y) {
  shape_stack.back()->location_ref().position_ref().y = y;
}

template <>
void object_bridge<data_staging::script>::set_z(double z) {
  shape_stack.back()->location_ref().set_z(z);
}

template <>
double object_bridge<data_staging::script>::get_vel_x() const {
  return shape_stack.back()->movement_ref().velocity_ref().x;
}

template <>
double object_bridge<data_staging::script>::get_vel_y() const {
  return shape_stack.back()->movement_ref().velocity_ref().y;
}

template <>
double object_bridge<data_staging::script>::get_velocity() const {
  return shape_stack.back()->movement_ref().velocity_speed();
}

template <>
void object_bridge<data_staging::script>::set_vel_x(double x) {
  shape_stack.back()->movement_ref().velocity_ref().x = x;
}

template <>
void object_bridge<data_staging::script>::set_vel_y(double y) {
  shape_stack.back()->movement_ref().velocity_ref().y = y;
}

template <>
void object_bridge<data_staging::script>::set_velocity(double vel) {
  shape_stack.back()->movement_ref().set_velocity_speed(vel);
}

template <>
object_bridge<data_staging::script>::object_bridge(generator *generator) : generator_(generator) {
  v8pp::class_<object_bridge> object_bridge_class(v8::Isolate::GetCurrent());
  object_bridge_class  // .template ctor<int>()
      .set("level", v8pp::property(&object_bridge::get_level))
      .set("unique_id", v8pp::property(&object_bridge::get_unique_id, &object_bridge::set_unique_id))
      .set("angle", v8pp::property(&object_bridge::get_angle, &object_bridge::set_angle))
      .set("opacity", v8pp::property(&object_bridge::get_opacity, &object_bridge::set_opacity))
      .set("mass", v8pp::property(&object_bridge::get_mass, &object_bridge::set_mass))
      .set("scale", v8pp::property(&object_bridge::get_scale, &object_bridge::set_scale))
      .set("x", v8pp::property(&object_bridge::get_x, &object_bridge::set_x))
      .set("y", v8pp::property(&object_bridge::get_y, &object_bridge::set_y))
      .set("z", v8pp::property(&object_bridge::get_z, &object_bridge::set_z))
      .set("vel_x", v8pp::property(&object_bridge::get_vel_x, &object_bridge::set_vel_x))
      .set("vel_y", v8pp::property(&object_bridge::get_vel_y, &object_bridge::set_vel_y))
      .set("velocity", v8pp::property(&object_bridge::get_velocity, &object_bridge::set_velocity))
      .set("props", v8pp::property(&object_bridge::get_properties_local_ref))
      .set("spawn", &object_bridge::spawn)
      .set("spawn3", &object_bridge::spawn3);
  instance_ = std::make_shared<v8::Persistent<v8::Object>>();
  (*instance_)
      .Reset(v8::Isolate::GetCurrent(), object_bridge_class.reference_external(v8::Isolate::GetCurrent(), this));
}

template class object_bridge<data_staging::script>;
