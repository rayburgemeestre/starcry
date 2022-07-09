/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "util/vector_logic.hpp"
#include "data/gradient.hpp"
#include "data/texture.hpp"
#include "data/blending_type.hpp"

#include <vector>
#include <string>

// merged into data namespace when ready
namespace data_staging {

class circle
{
private:
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
  circle(vector2d position, double radius, double radiussize)
  : position_(position), radius_(radius), radius_size_(radiussize) {}

  void set_gradient(std::string_view gradient) {
    gradient_ = gradient;
  }
};
}