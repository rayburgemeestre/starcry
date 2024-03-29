/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

namespace data_staging {
class generic {
private:
  double angle_ = 0;
  double rotate_ = 0;
  double scale_ = 1.;
  double recursive_scale_ = 1.;
  double opacity_ = 1.;
  double mass_ = 1;

public:
  double angle() const {
    return angle_;
  }
  double rotate() const {
    return rotate_;
  }
  double scale() const {
    return scale_;
  }
  double recursive_scale() const {
    return recursive_scale_;
  }
  double opacity() const {
    return opacity_;
  }
  double mass() const {
    return mass_;
  }
  void set_angle(double angle) {
    angle_ = angle;
  }
  void set_rotate(double rotate) {
    rotate_ = rotate;
  }
  void set_scale(double scale) {
    scale_ = scale;
  }
  void set_recursive_scale(double recursive_scale) {
    recursive_scale_ = recursive_scale;
  }
  void set_opacity(double opacity) {
    opacity_ = opacity;
  }
  void set_mass(double mass) {
    mass_ = mass;
  }
};
}  // namespace data_staging