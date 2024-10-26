/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator.h"
#include "interpreter/object_bridge.h"
#include "util/v8_interact.hpp"

template <>
int64_t object_bridge<data_staging::ellipse>::get_unique_id() const {
  return shape_stack.back()->meta_ref().unique_id();
}

template <>
std::string object_bridge<data_staging::ellipse>::get_random_hash() const {
  return shape_stack.back()->meta_ref().random_hash();
}

template <>
double object_bridge<data_staging::ellipse>::get_angle() const {
  return shape_stack.back()->generic_ref().angle();
}

template <>
double object_bridge<data_staging::ellipse>::get_rotate() const {
  return shape_stack.back()->generic_ref().rotate();
}

template <>
double object_bridge<data_staging::ellipse>::get_hue() const {
  return shape_stack.back()->styling_ref().hue();
}

template <>
double object_bridge<data_staging::ellipse>::get_opacity() const {
  return shape_stack.back()->generic_ref().opacity();
}

template <>
double object_bridge<data_staging::ellipse>::get_mass() const {
  return shape_stack.back()->generic_ref().mass();
}

template <>
double object_bridge<data_staging::ellipse>::get_scale() const {
  return shape_stack.back()->generic_ref().scale();
}

template <>
double object_bridge<data_staging::ellipse>::get_x() const {
  return shape_stack.back()->location_ref().position_ref().x;
}

template <>
double object_bridge<data_staging::ellipse>::get_y() const {
  return shape_stack.back()->location_ref().position_ref().y;
}

template <>
double object_bridge<data_staging::ellipse>::get_z() const {
  return shape_stack.back()->location_ref().z();
}

template <>
double object_bridge<data_staging::ellipse>::get_longest_diameter() const {
  return shape_stack.back()->longest_diameter();
}

template <>
double object_bridge<data_staging::ellipse>::get_shortest_diameter() const {
  return shape_stack.back()->shortest_diameter();
}

template <>
double object_bridge<data_staging::ellipse>::get_radius_size() const {
  return shape_stack.back()->radius_size();
}

template <>
int64_t object_bridge<data_staging::ellipse>::get_seed() const {
  return shape_stack.back()->styling_ref().seed();
}

template <>
int64_t object_bridge<data_staging::ellipse>::get_blending_type() const {
  return shape_stack.back()->styling_ref().get_blending_type().type();
}

template <>
std::string object_bridge<data_staging::ellipse>::get_gradient() const {
  return shape_stack.back()->styling_ref().gradient();
}

template <>
std::string object_bridge<data_staging::ellipse>::get_collision_group() const {
  return shape_stack.back()->behavior_ref().gravity_group();
}

template <>
std::string object_bridge<data_staging::ellipse>::get_gravity_group() const {
  return shape_stack.back()->behavior_ref().gravity_group();
}

template <>
std::string object_bridge<data_staging::ellipse>::get_unique_group() const {
  return shape_stack.back()->behavior_ref().unique_group();
}

template <>
void object_bridge<data_staging::ellipse>::set_unique_id(int64_t unique_id) {
  shape_stack.back()->meta_ref().set_unique_id(unique_id);
}

template <>
void object_bridge<data_staging::ellipse>::set_random_hash(const std::string& random_hash) {
  shape_stack.back()->meta_ref().set_random_hash(random_hash);
}

template <>
void object_bridge<data_staging::ellipse>::set_angle(double angle) {
  shape_stack.back()->generic_ref().set_angle(angle);
}

template <>
void object_bridge<data_staging::ellipse>::set_rotate(double rotate) {
  shape_stack.back()->generic_ref().set_rotate(rotate);
}

template <>
void object_bridge<data_staging::ellipse>::set_hue(double hue) {
  shape_stack.back()->styling_ref().set_hue(hue);
}

template <>
void object_bridge<data_staging::ellipse>::set_opacity(double opacity) {
  shape_stack.back()->generic_ref().set_opacity(opacity);
}

template <>
void object_bridge<data_staging::ellipse>::set_mass(double mass) {
  shape_stack.back()->generic_ref().set_mass(mass);
}

template <>
void object_bridge<data_staging::ellipse>::set_scale(double scale) {
  shape_stack.back()->generic_ref().set_scale(scale);
}
template <>
void object_bridge<data_staging::ellipse>::set_x(double x) {
  shape_stack.back()->location_ref().position_ref().x = x;
}

template <>
void object_bridge<data_staging::ellipse>::set_y(double y) {
  shape_stack.back()->location_ref().position_ref().y = y;
}

template <>
void object_bridge<data_staging::ellipse>::set_z(double z) {
  shape_stack.back()->location_ref().set_z(z);
}

template <>
void object_bridge<data_staging::ellipse>::set_longest_diameter(double longest_diameter) {
  shape_stack.back()->set_longest_diameter(longest_diameter);
}

template <>
void object_bridge<data_staging::ellipse>::set_shortest_diameter(double shortest_diameter) {
  shape_stack.back()->set_shortest_diameter(shortest_diameter);
}

template <>
void object_bridge<data_staging::ellipse>::set_radius_size(double radius_size) {
  shape_stack.back()->set_radius_size(radius_size);
}

template <>
double object_bridge<data_staging::ellipse>::get_vel_x() const {
  return shape_stack.back()->movement_ref().velocity_ref().x;
}

template <>
double object_bridge<data_staging::ellipse>::get_vel_y() const {
  return shape_stack.back()->movement_ref().velocity_ref().y;
}

template <>
double object_bridge<data_staging::ellipse>::get_velocity() const {
  return shape_stack.back()->movement_ref().velocity_speed();
}

template <>
void object_bridge<data_staging::ellipse>::set_vel_x(double x) {
  shape_stack.back()->movement_ref().velocity_ref().x = x;
}

template <>
void object_bridge<data_staging::ellipse>::set_vel_y(double y) {
  shape_stack.back()->movement_ref().velocity_ref().y = y;
}

template <>
void object_bridge<data_staging::ellipse>::set_velocity(double vel) {
  shape_stack.back()->movement_ref().set_velocity_speed(vel);
}

template <>
void object_bridge<data_staging::ellipse>::set_seed(int64_t new_value) const {
  return shape_stack.back()->styling_ref().set_seed(new_value);
}

template <>
void object_bridge<data_staging::ellipse>::set_blending_type(int64_t blending_type) const {
  return shape_stack.back()->styling_ref().set_blending_type(data::blending_type(blending_type));
}

template <>
void object_bridge<data_staging::ellipse>::set_gradient(const std::string& gradient) const {
  return shape_stack.back()->styling_ref().set_gradient(gradient);
}

template <>
void object_bridge<data_staging::ellipse>::set_collision_group(const std::string& cg) const {
  return shape_stack.back()->behavior_ref().set_collision_group(cg);
}

template <>
void object_bridge<data_staging::ellipse>::set_gravity_group(const std::string& gg) const {
  return shape_stack.back()->behavior_ref().set_gravity_group(gg);
}

template <>
void object_bridge<data_staging::ellipse>::set_unique_group(const std::string& ug) const {
  return shape_stack.back()->behavior_ref().set_unique_group(ug);
}

template <>
object_bridge<data_staging::ellipse>::object_bridge(interpreter::generator* generator,
                                                    interpreter::object_definitions& definitions)
    : generator_(generator), definitions_(definitions) {
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
      .property("longest_diameter", &object_bridge::get_longest_diameter, &object_bridge::set_longest_diameter)
      .property("shortest_diameter", &object_bridge::get_shortest_diameter, &object_bridge::set_shortest_diameter)
      .property("a", &object_bridge::get_longest_diameter, &object_bridge::set_longest_diameter)
      .property("b", &object_bridge::get_shortest_diameter, &object_bridge::set_shortest_diameter)
      .property("radiussize", &object_bridge::get_radius_size, &object_bridge::set_radius_size)
      .property("props", &object_bridge::get_properties_local_ref)
      .property("gradient", &object_bridge::get_gradient, &object_bridge::set_gradient)
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
      .function("spawn_parent", &object_bridge::spawn_parent)
      .function("destroy", &object_bridge::destroy)
      .property("seed", &object_bridge::get_seed, &object_bridge::set_seed)
      .property("blending_type", &object_bridge::get_blending_type, &object_bridge::set_blending_type)
      .property("collision_group", &object_bridge::get_collision_group, &object_bridge::set_collision_group)
      .property("gravity_group", &object_bridge::get_gravity_group, &object_bridge::set_gravity_group)
      .property("unique_group", &object_bridge::get_unique_group, &object_bridge::set_unique_group);

  instance_ = std::make_shared<v8::Persistent<v8::Object>>();
  (*instance_)
      .Reset(v8::Isolate::GetCurrent(), object_bridge_class.reference_external(v8::Isolate::GetCurrent(), this));
}

template class object_bridge<data_staging::ellipse>;
