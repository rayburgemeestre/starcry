/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "util/vector_logic.hpp"

namespace data_staging {
class movement {
private:
  vector2d velocity_ = {0, 0};
  double velocity_speed_ = 0;
  // TODO: add acceleration

public:
  vector2d velocity() const {
    return velocity_;
  }

  double velocity_speed() const {
    return velocity_speed_;
  }

  void set_velocity(vector2d new_value) {
    std::swap(velocity_, new_value);
  }

  vector2d& velocity_ref() {
    return velocity_;
  }

  void set_velocity(double x, double y, double velocity = 1.) {
    velocity_.x = x;
    velocity_.y = y;
    velocity_speed_ = velocity;
  }

  void set_velocity_speed(double velocity) {
    velocity_speed_ = velocity;
  }
};
}  // namespace data_staging