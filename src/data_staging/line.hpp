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

#include "data_staging/attrs.hpp"
#include "data_staging/behavior.hpp"
#include "data_staging/cascade.hpp"
#include "data_staging/generic.hpp"
#include "data_staging/location.hpp"
#include "data_staging/meta.hpp"
#include "data_staging/movement.hpp"
#include "data_staging/properties.hpp"
#include "data_staging/styling.hpp"
#include "data_staging/toroidal.hpp"

// merged into data namespace when ready
namespace data_staging {

class line {
private:
  meta meta_;
  location line_start_;
  location line_end_;
  location transitive_line_start_;
  location transitive_line_end_;
  movement movement_line_start_;
  movement movement_line_end_;

  double line_width_ = 1.;

  generic generic_;
  styling styling_;
  behavior behavior_;
  toroidal toroidal_;
  properties properties_;
  std::vector<cascade> cascades_in_;
  std::vector<cascade> cascades_out_;

  attrs attrs_;

public:
  line(std::string id, int64_t unique_id, vector2d line_start, vector2d line_end, double line_width)
      : meta_(std::move(id), unique_id), line_start_(line_start), line_end_(line_end), line_width_(line_width) {}

  const meta& meta_cref() const {
    return meta_;
  }
  data_staging::meta& meta_ref() {
    return meta_;
  }

  /**
   * Attributes data const reference
   */
  const attrs& attrs_cref() const {
    return attrs_;
  }

  /**
   * Attributes data reference
   */
  data_staging::attrs& attrs_ref() {
    return attrs_;
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
  data_staging::location& transitive_line_start_ref() {
    return transitive_line_start_;
  }
  data_staging::location& transitive_line_end_ref() {
    return transitive_line_end_;
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
  const generic& generic_cref() const {
    return generic_;
  }
  data_staging::generic& generic_ref() {
    return generic_;
  }
  const styling& styling_cref() const {
    return styling_;
  }
  data_staging::styling& styling_ref() {
    return styling_;
  }
  const behavior& behavior_cref() const {
    return behavior_;
  }
  data_staging::behavior& behavior_ref() {
    return behavior_;
  }
  const data_staging::toroidal& toroidal_cref() {
    return toroidal_;
  }
  data_staging::toroidal& toroidal_ref() {
    return toroidal_;
  }
  const properties& properties_cref() const {
    return properties_;
  }
  data_staging::properties& properties_ref() {
    return properties_;
  }
  double line_width() const {
    return line_width_;
  }
  void set_line_width(double line_width) {
    line_width_ = line_width;
  }
  void add_cascade_in(cascade_type cascade_type, int64_t unique_id) {
    cascades_in_.emplace_back(cascade_type, unique_id);
  }
  void add_cascade_out(cascade_type cascade_type, int64_t unique_id) {
    cascades_out_.emplace_back(cascade_type, unique_id);
  }
  const std::vector<cascade>& cascades_in() const {
    return cascades_in_;
  }
};
}  // namespace data_staging