/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>

namespace data_staging {
class toroidal {
private:
  std::string group_;
  double warp_width_;
  double warp_height_;
  double warp_dist_;

public:
  const std::string& group() const {
    return group_;
  }
  double warp_width() const {
    return warp_width_;
  }
  double warp_height() const {
    return warp_height_;
  }
  double warp_dist() const {
    return warp_dist_;
  }

  void set_group(const std::string& group) {
    group_ = group;
  }
  void set_warp_width(double warp_width) {
    warp_width_ = warp_width;
  }
  void set_warp_height(double warp_height) {
    warp_height_ = warp_height;
  }
  void set_warp_dist(double warp_dist) {
    warp_dist_ = warp_dist;
  }
};
}  // namespace data_staging
