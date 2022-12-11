/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "util/vector_logic.hpp"

#include <string>
#include <vector>

#include "data_staging/behavior.hpp"
#include "data_staging/generic.hpp"
#include "data_staging/location.hpp"
#include "data_staging/meta.hpp"
#include "data_staging/movement.hpp"
#include "data_staging/properties.hpp"
#include "data_staging/styling.hpp"
#include "data_staging/toroidal.hpp"

// merged into data namespace when ready
namespace data_staging {

class script {
private:
  meta meta_;
  location location_;
  location transitive_location_;
  movement movement_;

  generic generic_;
  behavior behavior_;
  toroidal toroidal_;
  properties properties_;
  styling styling_;

public:
  script(std::string id, int64_t unique_id, vector2d position) : meta_(std::move(id), unique_id), location_(position) {}

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
  const styling& styling_cref() const {
    return styling_;
  }
  data_staging::styling& styling_ref() {
    return styling_;
  }
};
}  // namespace data_staging
