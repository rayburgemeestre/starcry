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

public:
  object_bridge(int);

  void push_object(shape_class& c);
  void pop_object();

  void set_generator(native_generator* ptr);

  int64_t get_level() const;
  double get_x() const;
  double get_y() const;
  double get_z() const;
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
  void set_radius(double radius);
  void set_radius_size(double radiussize);

  v8::Local<v8::Object> get_properties_ref() const;

  void spawn(v8::Local<v8::Object> obj);

  static void add_to_context(v8pp::context& context);
};

extern object_bridge<data_staging::circle>* object_bridge_circle;
extern object_bridge<data_staging::line>* object_bridge_line;
