/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "interpreter/object_bridge.h"
#include "util/v8_interact.hpp"

template <>
int64_t object_bridge<data_staging::text>::get_unique_id() const {
  return shape_stack.back()->meta_ref().unique_id();
}

template <>
std::string object_bridge<data_staging::text>::get_random_hash() const {
  return shape_stack.back()->meta_ref().random_hash();
}

template <>
double object_bridge<data_staging::text>::get_angle() const {
  return shape_stack.back()->generic_ref().angle();
}

template <>
double object_bridge<data_staging::text>::get_rotate() const {
  return shape_stack.back()->generic_ref().rotate();
}

template <>
double object_bridge<data_staging::text>::get_hue() const {
  return shape_stack.back()->styling_ref().hue();
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
void object_bridge<data_staging::text>::set_random_hash(const std::string& random_hash) {
  shape_stack.back()->meta_ref().set_random_hash(random_hash);
}

template <>
void object_bridge<data_staging::text>::set_angle(double angle) {
  shape_stack.back()->generic_ref().set_angle(angle);
}

template <>
void object_bridge<data_staging::text>::set_rotate(double rotate) {
  shape_stack.back()->generic_ref().set_rotate(rotate);
}

template <>
void object_bridge<data_staging::text>::set_hue(double hue) {
  shape_stack.back()->styling_ref().set_hue(hue);
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
object_bridge<data_staging::text>::object_bridge(interpreter::object_definitions& definitions,
                                                 interpreter::spawner& spawner)
    : object_definitions_(definitions), spawner_(spawner) {
  v8pp::class_<object_bridge> object_bridge_class(v8::Isolate::GetCurrent());
  // @add_field@
  object_bridge_class  // .template ctor<int>()
      .property("level", &object_bridge::get_level)
      .property("unique_id", &object_bridge::get_unique_id, &object_bridge::set_unique_id)
      .property("random_hash", &object_bridge::get_random_hash, &object_bridge::set_random_hash)
      .property("angle", &object_bridge::get_angle, &object_bridge::set_angle)
      .property("rotate", &object_bridge::get_rotate, &object_bridge::set_rotate)
      .property("hue", &object_bridge::get_hue, &object_bridge::set_hue)
      .property("opacity", &object_bridge::get_opacity, &object_bridge::set_opacity)
      .property("mass", &object_bridge::get_mass, &object_bridge::set_mass)
      .property("scale", &object_bridge::get_scale, &object_bridge::set_scale)
      .property("recursive_scale", &object_bridge::get_recursive_scale, &object_bridge::set_recursive_scale)
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
      .property("texture", &object_bridge::get_texture, &object_bridge::set_texture)
      .property("texture_3d", &object_bridge::get_texture_3d, &object_bridge::set_texture_3d)
      .property("texture_offset_x", &object_bridge::get_texture_offset_x, &object_bridge::set_texture_offset_x)
      .property("texture_offset_y", &object_bridge::get_texture_offset_y, &object_bridge::set_texture_offset_y)
      .property("texture_effect", &object_bridge::get_texture_effect, &object_bridge::set_texture_effect)
      .property("zernike_type", &object_bridge::get_zernike_type, &object_bridge::set_zernike_type)
      .function("attr", &object_bridge::get_attr)
      .function("set_attr", &object_bridge::set_attr)
      .function("spawn", &object_bridge::spawn)
      .function("spawn2", &object_bridge::spawn2)
      .function("spawn3", &object_bridge::spawn3)
      .function("destroy", &object_bridge::destroy);
  instance_ = std::make_shared<v8::Persistent<v8::Object>>();
  (*instance_)
      .Reset(v8::Isolate::GetCurrent(), object_bridge_class.reference_external(v8::Isolate::GetCurrent(), this));
}

template class object_bridge<data_staging::text>;
