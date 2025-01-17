/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <memory>
#include <vector>

#include "data_staging/shape.hpp"

#include "v8.h"

namespace interpreter {
class generator;
class object_definitions;
class spawner;
}  // namespace interpreter

template <typename shape_class>
class object_bridge {
  std::vector<shape_class*> shape_stack;

  interpreter::object_definitions& object_definitions_;
  interpreter::spawner& spawner_;
  mutable bool properties_accessed_ = false;
  mutable bool gradients_accessed_ = false;
  std::shared_ptr<v8::Persistent<v8::Object>> instance_ = nullptr;

public:
  explicit object_bridge(interpreter::object_definitions& definitions, interpreter::spawner& spawner_);

  void push_object(shape_class& c);
  void pop_object();

  // @add_field@

  int64_t get_level() const;
  int64_t get_unique_id() const;
  std::string get_random_hash() const;
  double get_angle() const;
  double get_rotate() const;
  double get_hue() const;
  double get_opacity() const;
  double get_mass() const;
  double get_scale() const;
  double get_x() const;
  double get_y() const;
  double get_z() const;
  double get_vel_x() const;
  double get_vel_y() const;
  double get_velocity() const;
  double get_x2() const;
  double get_y2() const;
  double get_z2() const;
  double get_radius() const;
  double get_longest_diameter() const;
  double get_shortest_diameter() const;
  double get_radius_size() const;
  const std::string& get_text() const;
  double get_text_size() const;
  const std::string& get_text_align() const;
  bool get_text_fixed() const;
  int64_t get_seed() const;
  std::string get_collision_group() const;
  std::string get_gravity_group() const;
  std::string get_unique_group() const;
  int64_t get_blending_type() const;
  std::string get_gradient() const;
  std::string get_texture() const;
  data::texture_3d get_texture_3d() const;
  data::zernike_type get_zernike_type() const;
  data::texture_effect get_texture_effect() const;
  double get_texture_offset_x() const;
  double get_texture_offset_y() const;
  double get_recursive_scale() const;

  void set_unique_id(int64_t unique_id);
  void set_random_hash(const std::string& random_hash);
  void set_angle(double angle);
  void set_rotate(double rotate);
  void set_hue(double hue);
  void set_opacity(double opacity);
  void set_mass(double mass);
  void set_scale(double scale);
  void set_x(double x);
  void set_y(double y);
  void set_z(double z);
  void set_x2(double x);
  void set_y2(double y);
  void set_z2(double z);
  void set_vel_x(double x);
  void set_vel_y(double y);
  void set_velocity(double vel);
  void set_radius(double radius);
  void set_longest_diameter(double diameter);
  void set_shortest_diameter(double diameter);
  void set_radius_size(double radiussize);
  void set_text(const std::string& text);
  void set_text_size(double size);
  void set_text_align(const std::string& align);
  void set_text_fixed(bool fixed);
  void set_seed(int64_t) const;
  void set_collision_group(const std::string& group) const;
  void set_gravity_group(const std::string& group) const;
  void set_unique_group(const std::string& group) const;
  void set_blending_type(int64_t new_type) const;
  void set_gradient(const std::string& gradient) const;
  void set_texture(const std::string& value) const;
  void set_texture_3d(data::texture_3d value) const;
  void set_zernike_type(data::zernike_type value) const;
  void set_texture_effect(data::texture_effect value) const;
  void set_texture_offset_x(double x) const;
  void set_texture_offset_y(double y) const;
  void set_recursive_scale(double value) const;

  v8::Persistent<v8::Object>& get_properties_ref() const;
  v8::Local<v8::Object> get_properties_local_ref() const;
  v8::Local<v8::Array> get_gradients_local_ref() const;

  std::vector<std::tuple<double, std::string>>& get_gradients_ref() const;

  v8::Local<v8::Value> get_attr(v8::Local<v8::String> obj);
  void set_attr(v8::Local<v8::String> key, v8::Local<v8::String> value);
  int64_t spawn(v8::Local<v8::Object> obj);
  // TODO: rename to spawn_line
  int64_t spawn2(v8::Local<v8::Object> line_obj, int64_t obj1);
  int64_t spawn3(v8::Local<v8::Object> line_obj, int64_t obj1, int64_t obj2);
  int64_t spawn_parent(v8::Local<v8::Object> obj);
  int64_t destroy();

  v8::Persistent<v8::Object>& instance();
};
