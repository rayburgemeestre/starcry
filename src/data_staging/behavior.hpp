/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>

namespace data_staging {
class behavior {
private:
  std::string collision_group_;
  std::string gravity_group_;
  std::string toroidal_group_;

  int64_t last_collide_ = -1;

public:
  std::string collision_group() const {
    return collision_group_;
  }
  std::string gravity_group() const {
    return gravity_group_;
  }
  std::string toroidal_group() const {
    return toroidal_group_;
  }
  const std::string& collision_group_ref() const {
    return collision_group_;
  }
  const std::string& gravity_group_ref() const {
    return gravity_group_;
  }
  const std::string& toroidal_group_ref() const {
    return toroidal_group_;
  }
  int64_t last_collide() const {
    return last_collide_;
  }
  void set_last_collide(int64_t last_collide) {
    last_collide_ = last_collide;
  }
  void set_collision_group(std::string collision_group) {
    collision_group_ = collision_group;
  }
  void set_gravity_group(std::string gravity_group) {
    gravity_group_ = gravity_group;
  }
  void set_toroidal_group(std::string toroidal_group) {
    toroidal_group_ = toroidal_group;
  }
};
}  // namespace data_staging