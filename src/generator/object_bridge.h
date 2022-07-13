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

class object_bridge {
private:
  std::vector<data_staging::circle*> circle;
  native_generator* generator_ = nullptr;

public:
  object_bridge(int);

  void push_object(data_staging::circle& c);
  void pop_object();

  void set_generator(native_generator* ptr);

  double get_x() const;
  double get_y() const;
  double get_z() const;
  double get_radius() const;
  double get_radius_size() const;

  void set_x(double x);
  void set_y(double y);
  void set_z(double z);
  void set_radius(double radius);
  void set_radius_size(double radiussize);

  void spawn(v8::Local<v8::Object> obj);

  static void add_to_context(v8pp::context& context);
};

extern object_bridge* object_bridge_ptr;