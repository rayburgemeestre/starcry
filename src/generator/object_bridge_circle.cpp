/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator.h"
#include "generator/object_bridge.h"
#include "util/v8_interact.hpp"

template <>
int64_t object_bridge<data_staging::circle>::get_unique_id() const {
  return shape_stack.back()->meta_ref().unique_id();
}

template <>
double object_bridge<data_staging::circle>::get_angle() const {
  return shape_stack.back()->generic_ref().angle();
}

template <>
double object_bridge<data_staging::circle>::get_opacity() const {
  return shape_stack.back()->generic_ref().opacity();
}

template <>
double object_bridge<data_staging::circle>::get_mass() const {
  return shape_stack.back()->generic_ref().mass();
}

template <>
double object_bridge<data_staging::circle>::get_scale() const {
  return shape_stack.back()->generic_ref().scale();
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
int64_t object_bridge<data_staging::circle>::get_seed() const {
  return shape_stack.back()->styling_ref().seed();
}

template <>
int64_t object_bridge<data_staging::circle>::get_blending_type() const {
  return shape_stack.back()->styling_ref().get_blending_type().type();
}

template <>
std::string object_bridge<data_staging::circle>::get_collision_group() const {
  return shape_stack.back()->behavior_ref().gravity_group();
}

template <>
std::string object_bridge<data_staging::circle>::get_gravity_group() const {
  return shape_stack.back()->behavior_ref().gravity_group();
}

template <>
void object_bridge<data_staging::circle>::set_unique_id(int64_t unique_id) {
  shape_stack.back()->meta_ref().set_unique_id(unique_id);
}

template <>
void object_bridge<data_staging::circle>::set_angle(double angle) {
  shape_stack.back()->generic_ref().set_angle(angle);
}

template <>
void object_bridge<data_staging::circle>::set_opacity(double opacity) {
  shape_stack.back()->generic_ref().set_opacity(opacity);
}

template <>
void object_bridge<data_staging::circle>::set_mass(double mass) {
  shape_stack.back()->generic_ref().set_mass(mass);
}

template <>
void object_bridge<data_staging::circle>::set_scale(double scale) {
  shape_stack.back()->generic_ref().set_scale(scale);
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
double object_bridge<data_staging::circle>::get_vel_x() const {
  return shape_stack.back()->movement_ref().velocity_ref().x;
}

template <>
double object_bridge<data_staging::circle>::get_vel_y() const {
  return shape_stack.back()->movement_ref().velocity_ref().y;
}

template <>
double object_bridge<data_staging::circle>::get_velocity() const {
  return shape_stack.back()->movement_ref().velocity_speed();
}

template <>
void object_bridge<data_staging::circle>::set_vel_x(double x) {
  shape_stack.back()->movement_ref().velocity_ref().x = x;
}

template <>
void object_bridge<data_staging::circle>::set_vel_y(double y) {
  shape_stack.back()->movement_ref().velocity_ref().y = y;
}

template <>
void object_bridge<data_staging::circle>::set_velocity(double vel) {
  shape_stack.back()->movement_ref().set_velocity_speed(vel);
}

template <>
void object_bridge<data_staging::circle>::set_seed(int64_t new_value) const {
  return shape_stack.back()->styling_ref().set_seed(new_value);
}

template <>
void object_bridge<data_staging::circle>::set_blending_type(int64_t blending_type) const {
  return shape_stack.back()->styling_ref().set_blending_type(data::blending_type(blending_type));
}

template <>
void object_bridge<data_staging::circle>::set_collision_group(const std::string& cg) const {
  return shape_stack.back()->behavior_ref().set_collision_group(cg);
}

template <>
void object_bridge<data_staging::circle>::set_gravity_group(const std::string& gg) const {
  return shape_stack.back()->behavior_ref().set_gravity_group(gg);
}

template <>
object_bridge<data_staging::circle>::object_bridge(generator* generator) : generator_(generator) {
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
      .property("radius", &object_bridge::get_radius, &object_bridge::set_radius)
      .property("radiussize", &object_bridge::get_radius_size, &object_bridge::set_radius_size)
      .property("props", &object_bridge::get_properties_local_ref)
      .property("gradients", &object_bridge::get_gradients_local_ref)
      .function("spawn", &object_bridge::spawn)
      .function("spawn2", &object_bridge::spawn2)
      .function("spawn3", &object_bridge::spawn3)
      .function("spawn_parent", &object_bridge::spawn_parent)
      .property("seed", &object_bridge::get_seed, &object_bridge::set_seed)
      .property("blending_type", &object_bridge::get_blending_type, &object_bridge::set_blending_type)
      .property("collision_group", &object_bridge::get_collision_group, &object_bridge::set_collision_group)
      .property("gravity_group", &object_bridge::get_gravity_group, &object_bridge::set_gravity_group);

  instance_ = std::make_shared<v8::Persistent<v8::Object>>();
  (*instance_)
      .Reset(v8::Isolate::GetCurrent(), object_bridge_class.reference_external(v8::Isolate::GetCurrent(), this));
}

template class object_bridge<data_staging::circle>;