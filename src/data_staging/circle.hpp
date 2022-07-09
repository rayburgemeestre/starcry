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

#include <string>
#include <vector>

// merged into data namespace when ready
namespace data_staging {

class circle {
private:
  int64_t unique_id_;

  vector2d position_;
  double z_ = 0;

  double radius_ = 100.;
  double radius_size_ = 1.;

  double scale_ = 1.;
  double opacity_ = 1.;

  std::string gradient_;
  std::vector<std::pair<double, data::gradient>> gradients_;
  std::vector<std::pair<double, data::texture>> textures_;

  data::blending_type blending_type_ = data::blending_type::normal;

  std::string collision_group_;
  std::string gravity_group_;
  std::string toroidal_group_;

public:
  circle(int64_t unique_id, vector2d position, double radius, double radiussize)
      : unique_id_(unique_id), position_(position), radius_(radius), radius_size_(radiussize) {}

  int64_t unique_id() const {
    return unique_id_;
  }

  vector2d position() const {
    return position_;
  }

  void set_gradient(std::string_view gradient) {
    gradient_ = gradient;
  }

  double radius() {
    return radius_;
  }
  double radius_size() {
    return radius_size_;
  }
  double scale() {
    return scale_;
  }
  double opacity() {
    return opacity_;
  }
  std::string gradient() {
    return gradient_;
  }
  data::blending_type blending_type() {
    return blending_type_;
  }
  std::string collision_group() {
    return collision_group_;
  }
  std::string gravity_group() {
    return gravity_group_;
  }
  std::string toroidal_group() {
    return toroidal_group_;
  }
};
}  // namespace data_staging