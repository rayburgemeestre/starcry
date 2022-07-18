/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <memory>
#include <vector>

#include "data_staging/shape.hpp"
#include "util/v8_wrapper.hpp"

#include "v8.h"

class native_generator;

template <typename shape_class>
class object_bridge {
private:
  std::vector<shape_class*> shape_stack;
  native_generator* generator_ = nullptr;
  mutable bool properties_accessed_ = false;
  mutable bool gradients_accessed_ = false;
  std::shared_ptr<v8::Persistent<v8::Object>> instance_ = nullptr;

public:
  object_bridge(native_generator* generator);

  void push_object(shape_class& c);
  void pop_object();

  int64_t get_level() const;
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
  double get_radius_size() const;

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
  void set_radius_size(double radiussize);

  v8::Persistent<v8::Object>& get_properties_ref() const;
  v8::Local<v8::Object> get_properties_local_ref() const;
  v8::Local<v8::Array> get_gradients_local_ref() const;

  std::vector<std::tuple<double, std::string>>& get_gradients_ref() const;
  // void set_gradients(std::vector<std::tuple<double, std::string>> gradients);

  void spawn(v8::Local<v8::Object> obj);

  v8::Persistent<v8::Object>& instance();
};