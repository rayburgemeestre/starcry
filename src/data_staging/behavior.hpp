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
};
}  // namespace data_staging