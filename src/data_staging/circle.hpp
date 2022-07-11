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
  std::string namespace_;
  std::string id_;
  int64_t unique_id_;
  int64_t level_;

  vector2d position_ = {0, 0};
  // double z_ = 0;
  vector2d velocity_ = {0, 0};
  double velocity_speed_ = 0;

  double radius_ = 100.;
  double radius_size_ = 1.;

  double angle_ = 0;
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
  circle(std::string id, int64_t unique_id, vector2d position, double radius, double radiussize)
      : id_(std::move(id)), unique_id_(unique_id), position_(position), radius_(radius), radius_size_(radiussize) {}

  const std::string& namespace_name() const {
    return namespace_;
  }

  const std::string& id() const {
    return id_;
  }

  int64_t unique_id() const {
    return unique_id_;
  }

  int64_t level() const {
    return level_;
  }

  vector2d position() {
    return position_;
  }

  vector2d& position_ref() {
    return position_;
  }
  const vector2d& position_cref() const {
    return position_;
  }

  vector2d velocity() const {
    return velocity_;
  }

  double velocity_speed() const {
    return velocity_speed_;
  }

  double radius() const {
    return radius_;
  }
  double radius_size() const {
    return radius_size_;
  }
  double angle() const {
    return angle_;
  }
  double scale() const {
    return scale_;
  }
  double opacity() const {
    return opacity_;
  }
  std::string gradient() const {
    return gradient_;
  }
  data::blending_type blending_type() const {
    return blending_type_;
  }
  std::string collision_group() const {
    return collision_group_;
  }
  std::string gravity_group() const {
    return gravity_group_;
  }
  std::string toroidal_group() const {
    return toroidal_group_;
  }

  void set_velocity(double x, double y, double velocity = 1.) {
    velocity_.x = x;
    velocity_.y = y;
    velocity_speed_ = velocity;
  }

  void set_gradient(std::string_view gradient) {
    gradient_ = gradient;
  }
};
}  // namespace data_staging