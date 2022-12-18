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

#include "data_staging/behavior.hpp"
#include "data_staging/cascade.hpp"
#include "data_staging/generic.hpp"
#include "data_staging/location.hpp"
#include "data_staging/meta.hpp"
#include "data_staging/movement.hpp"
#include "data_staging/properties.hpp"
#include "data_staging/styling.hpp"
#include "data_staging/toroidal.hpp"

#include <string>
#include <vector>

// merged into data namespace when ready
namespace data_staging {

class text {
private:
  meta meta_;
  location location_;
  location transitive_location_;
  movement movement_;

  std::string font_name_;
  std::string text_;
  double text_size_ = 10;
  std::string text_align_ = "center";
  bool text_fixed_ = false;

  generic generic_;
  styling styling_;
  behavior behavior_;
  toroidal toroidal_;
  properties properties_;
  std::vector<cascade> cascades_in_;
  std::vector<cascade> cascades_out_;

public:
  text(std::string id,
       int64_t unique_id,
       vector2d position,
       std::string text,
       double text_size,
       std::string text_align,
       bool text_fixed)
      : meta_(std::move(id), unique_id),
        location_(position),
        text_(std::move(text)),
        text_size_(text_size),
        text_align_(std::move(text_align)),
        text_fixed_(text_fixed) {}

  const meta& meta_cref() const {
    return meta_;
  }
  data_staging::meta& meta_ref() {
    return meta_;
  }
  const location& location_cref() const {
    return location_;
  }
  data_staging::location& location_ref() {
    return location_;
  }
  data_staging::location& transitive_location_ref() {
    return transitive_location_;
  }
  const movement& movement_cref() const {
    return movement_;
  }
  data_staging::movement& movement_ref() {
    return movement_;
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

  const std::string& text_cref() const {
    return text_;
  }

  void set_font_name(std::string font_name) {
    font_name_ = std::move(font_name);
  }

  const std::string& font_name() const {
    return font_name_;
  }

  void set_text(const std::string& text) {
    text_ = text;
  }

  double text_size() const {
    return text_size_;
  }

  void set_text_size(double text_size) {
    text_size_ = text_size;
  }

  const std::string& text_align() const {
    return text_align_;
  }

  void set_text_align(const std::string& text_align) {
    text_align_ = text_align;
  }

  bool text_fixed() const {
    return text_fixed_;
  }

  void set_text_fixed(bool text_fixed) {
    text_fixed_ = text_fixed;
  }

  const std::vector<cascade>& cascade_out_cref() const {
    return cascades_out_;
  }

  void add_cascade_in(cascade_type cascade_type, int64_t unique_id) {
    cascades_in_.emplace_back(cascade_type, unique_id);
  }

  void add_cascade_out(cascade_type cascade_type, int64_t unique_id) {
    cascades_out_.emplace_back(cascade_type, unique_id);
  }
};
}  // namespace data_staging
