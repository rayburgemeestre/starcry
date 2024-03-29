/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "util/vector_logic.hpp"

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

#include <string>
#include <vector>

// merged into data namespace when ready
namespace data_staging {

class circle {
  meta meta_;
  location location_;
  location transitive_location_;
  movement movement_;

  double radius_ = 100.;
  double radius_size_ = 1.;

  generic generic_;
  styling styling_;
  behavior behavior_;
  toroidal toroidal_;
  properties properties_;
  std::vector<cascade> cascades_in_;
  std::vector<cascade> cascades_out_;

  attrs attrs_;

public:
  /**
   * Constructor
   * @param id textual identifier usually the name of the object it instantiates
   * @param unique_id unique numeric identifier
   * @param position x, y, z coordinates
   * @param radius radius of the circle
   * @param radiussize half of the circle width
   */
  circle(std::string id, int64_t unique_id, vector2d position, double radius, double radiussize)
      : meta_(std::move(id), unique_id), location_(position), radius_(radius), radius_size_(radiussize) {}

  /**
   * Meta data const reference
   */
  [[nodiscard]] const meta& meta_cref() const {
    return meta_;
  }
  /**
   * Meta data reference
   */
  meta& meta_ref() {
    return meta_;
  }

  /**
   * Attributes data const reference
   */
  [[nodiscard]] const attrs& attrs_cref() const {
    return attrs_;
  }

  /**
   * Attributes data reference
   */
  attrs& attrs_ref() {
    return attrs_;
  }

  /**
   * Location data const reference
   */
  [[nodiscard]] const location& location_cref() const {
    return location_;
  }
  /**
   * Location data reference
   */
  location& location_ref() {
    return location_;
  }
  /**
   * Absolute location data reference
   */
  location& transitive_location_ref() {
    return transitive_location_;
  }
  /**
   * Absolute location data const reference
   */
  const location& transitive_location_cref() {
    return transitive_location_;
  }
  /**
   * Movement data const reference
   */
  [[nodiscard]] const movement& movement_cref() const {
    return movement_;
  }
  /**
   * Movement data reference
   */
  movement& movement_ref() {
    return movement_;
  }
  /**
   * Generic data const reference
   */
  [[nodiscard]] const generic& generic_cref() const {
    return generic_;
  }
  /**
   * Generic data reference
   */
  generic& generic_ref() {
    return generic_;
  }

  /**
   * Styling data const reference
   */
  [[nodiscard]] const styling& styling_cref() const {
    return styling_;
  }
  /**
   * Styling data reference
   */
  styling& styling_ref() {
    return styling_;
  }
  /**
   * Behavior data const reference
   */
  [[nodiscard]] const behavior& behavior_cref() const {
    return behavior_;
  }
  /**
   * Behavior data reference
   */
  behavior& behavior_ref() {
    return behavior_;
  }
  /**
   * Toroidal data const reference
   */
  const toroidal& toroidal_cref() {
    return toroidal_;
  }
  /**
   * Toroidal data reference
   */
  toroidal& toroidal_ref() {
    return toroidal_;
  }
  /**
   * Properties data const reference
   */
  [[nodiscard]] const properties& properties_cref() const {
    return properties_;
  }
  /**
   * Properties data reference
   */
  properties& properties_ref() {
    return properties_;
  }

  /**
   * Radius
   */
  [[nodiscard]] double radius() const {
    return radius_;
  }
  /**
   * Radius size
   */
  [[nodiscard]] double radius_size() const {
    return radius_size_;
  }
  /**
   * Set radius
   * @param radius
   */
  void set_radius(double radius) {
    radius_ = radius;
  }
  /**
   * Set radius size
   * @param radius_size
   */
  void set_radius_size(double radius_size) {
    radius_size_ = radius_size;
  }

  /**
   * Cascades in const reference
   */
  [[nodiscard]] const std::vector<cascade>& cascade_out_cref() const {
    return cascades_out_;
  }

  /**
   * Add cascade in link
   * @param cascade_type
   * @param unique_id
   */
  void add_cascade_in(cascade_type cascade_type, int64_t unique_id) {
    cascades_in_.emplace_back(cascade_type, unique_id);
  }

  /**
   * Add cascade out link
   * @param cascade_type
   * @param unique_id
   */
  void add_cascade_out(cascade_type cascade_type, int64_t unique_id) {
    cascades_out_.emplace_back(cascade_type, unique_id);
  }
};
}  // namespace data_staging