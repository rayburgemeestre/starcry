/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "data/blending_type.hpp"
#include "data/gradient.hpp"

namespace data_staging {
class styling {
private:
  std::string gradient_;
  std::vector<std::pair<double, data::gradient>> gradients_;
  std::vector<std::pair<double, data::texture>> textures_;
  data::blending_type blending_type_ = data::blending_type::normal;

public:
  std::string gradient() const {
    return gradient_;
  }
  data::blending_type blending_type() const {
    return blending_type_;
  }

  void set_gradient(std::string_view gradient) {
    gradient_ = gradient;
  }

  std::vector<std::pair<double, data::gradient>>& gradients_ref() {
    return gradients_;
  }
};
}  // namespace data_staging