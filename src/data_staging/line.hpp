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

class line {
private:
  vector2d line_start_;
  vector2d line_end_;
  // double z_ = 0;
  // double z2_ = 0;

  double line_width_ = 1.;

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
  line(vector2d line_start, vector2d line_end, double line_width)
      : line_start_(line_start), line_end_(line_end), line_width_(line_width) {}

  void set_gradient(std::string_view gradient) {
    gradient_ = gradient;
  }
};
}  // namespace data_staging