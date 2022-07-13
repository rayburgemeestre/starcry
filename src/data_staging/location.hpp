/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "util/vector_logic.hpp"

namespace data_staging {
class location {
private:
  vector2d position_ = {0, 0};
  double z_ = 0;

public:
  location();

  location(vector2d position, double z = 0) : position_(std::move(position)), z_(z) {}

  vector2d position() {
    return position_;
  }

  vector2d& position_ref() {
    return position_;
  }

  const vector2d& position_cref() const {
    return position_;
  }

  double z() const {
    return z_;
  }

  void set_z(double z) {
    z_ = z;
  }
};
}  // namespace data_staging