/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "data/blending_type.hpp"
#include "data/gradient.hpp"
#include "data/texture.hpp"
#include "util/vector_logic.hpp"

#include "data_staging/behavior.hpp"
#include "data_staging/generic.hpp"
#include "data_staging/location.hpp"
#include "data_staging/meta.hpp"
#include "data_staging/movement.hpp"
#include "data_staging/styling.hpp"

#include <string>
#include <vector>

// merged into data namespace when ready
namespace data_staging {

class circle {
private:
  meta meta_;
  location location_;
  movement movement_;

  double radius_ = 100.;
  double radius_size_ = 1.;

  generic generic_;
  styling styling_;
  behavior behavior_;

public:
  circle(std::string id, int64_t unique_id, vector2d position, double radius, double radiussize)
      : meta_(std::move(id), unique_id), location_(position), radius_(radius), radius_size_(radiussize) {}

  const meta& meta() const {
    return meta_;
  }
  data_staging::meta& meta_ref() {
    return meta_;
  }
  const location& location() const {
    return location_;
  }
  data_staging::location& location_ref() {
    return location_;
  }
  const movement& movement() const {
    return movement_;
  }
  data_staging::movement& movement_ref() {
    return movement_;
  }
  const generic& generic() const {
    return generic_;
  }
  data_staging::generic& generic_ref() {
    return generic_;
  }
  const styling& styling() const {
    return styling_;
  }
  data_staging::styling& styling_ref() {
    return styling_;
  }
  const behavior& behavior() const {
    return behavior_;
  }
  data_staging::behavior& behavior_ref() {
    return behavior_;
  }

  double radius() const {
    return radius_;
  }
  double radius_size() const {
    return radius_size_;
  }
  void set_radius(double radius) {
    radius_ = radius;
  }
  void set_radius_size(double radius_size) {
    radius_size_ = radius_size;
  }
};
}  // namespace data_staging