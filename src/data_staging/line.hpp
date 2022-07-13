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

#include "data_staging/behavior.hpp"
#include "data_staging/generic.hpp"
#include "data_staging/location.hpp"
#include "data_staging/meta.hpp"
#include "data_staging/movement.hpp"
#include "data_staging/styling.hpp"

// merged into data namespace when ready
namespace data_staging {

class line {
private:
  meta meta_;
  location line_start_;
  location line_end_;
  movement movement_line_start_;
  movement movement_line_end_;

  double line_width_ = 1.;

  generic generic_;
  styling styling_;
  behavior behavior_;

public:
  line(std::string id, int64_t unique_id, vector2d line_start, vector2d line_end, double line_width)
      : meta_(std::move(id), unique_id), line_start_(line_start), line_end_(line_end), line_width_(line_width) {}

  const meta& meta() const {
    return meta_;
  }
  data_staging::meta& meta_ref() {
    return meta_;
  }
  const location& line_start() const {
    return line_start_;
  }
  data_staging::location& line_start_ref() {
    return line_start_;
  }
  const location& line_end() const {
    return line_end_;
  }
  data_staging::location& line_end_ref() {
    return line_end_;
  }
  const movement& movement_line_start() const {
    return movement_line_start_;
  }
  data_staging::movement& movement_line_start_ref() {
    return movement_line_start_;
  }
  const movement& movement_line_end() const {
    return movement_line_end_;
  }
  data_staging::movement& movement_line_end_ref() {
    return movement_line_end_;
  }
  const generic& generic() const {
    return generic_;
  }
  data_staging::generic& generic_ref() {
    return generic_;
  }
  const styling& styling() const {
    return styling_;
  }
  data_staging::styling& styling_ref() {
    return styling_;
  }
  const behavior& behavior() const {
    return behavior_;
  }
  data_staging::behavior& behavior_ref() {
    return behavior_;
  }
  double line_width() const {
    return line_width_;
  }
  void set_line_width(double line_width) {
    line_width_ = line_width;
  }
};
}  // namespace data_staging