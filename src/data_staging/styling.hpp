/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "data/blending_type.hpp"
// #include "data/gradient.hpp"

#include "util/logger.h"

namespace data_staging {
class styling {
private:
  std::string gradient_;
  std::vector<std::tuple<double, std::string>> gradients_;
  // std::vector<std::pair<double, data::gradient>> real_gradients_;
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

  decltype(gradients_)& get_gradients_ref() {
    return gradients_;
  }

  const decltype(gradients_)& get_gradients_cref() const {
    return gradients_;
  }

  void set_gradients(std::vector<std::tuple<double, std::string>>& gradients) {
    gradients_ = gradients;
  }

  //  void commit() {
  //    logger(INFO) << "Real gradients listing: " << std::endl;
  //    for (const auto& [key, value] : real_gradients_) {
  //      logger(INFO) << " ~~ " << key << ": " << value.colors.size() << std::endl;
  //    }
  //
  //    for (const auto& [key, value] : gradients_) {
  //      logger(INFO) << " ~~ " << key << ": " << value << std::endl;
  //    }
  //
  //    logger(INFO) << "Real gradients after: " << std::endl;
  //    for (const auto& [key, value] : real_gradients_) {
  //      logger(INFO) << " ~~ " << key << ": " << value.colors.size() << std::endl;
  //    }
  //
  //  }
};
}  // namespace data_staging