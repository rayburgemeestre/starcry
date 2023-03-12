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

class ellipse {
private:
  meta meta_;
  location location_;
  location transitive_location_;
  movement movement_;

  double longest_diameter_ = 100;
  double shortest_diameter_ = 100;
  double radius_size_ = 1.;

  generic generic_;
  styling styling_;
  behavior behavior_;
  toroidal toroidal_;
  properties properties_;
  std::vector<cascade> cascades_in_;
  std::vector<cascade> cascades_out_;

public:
  /**
   * Constructor
   * @param id textual identifier usually the name of the object it instantiates
   * @param unique_id unique numeric identifier
   * @param position x, y, z coordinates
   * @param longest_diameter a
   * @param shortest_diameter b
   * @param radiussize half of the ellipse width
   */
  ellipse(std::string id,
          int64_t unique_id,
          vector2d position,
          double longest_diameter,
          double shortest_diameter,
          double radiussize)
      : meta_(std::move(id), unique_id),
        location_(position),
        longest_diameter_(longest_diameter),
        shortest_diameter_(shortest_diameter),
        radius_size_(radiussize) {}

  /**
   * Meta data const reference
   */
  const meta& meta_cref() const {
    return meta_;
  }
  /**
   * Meta data reference
   */
  data_staging::meta& meta_ref() {
    return meta_;
  }
  /**
   * Location data const reference
   */
  const location& location_cref() const {
    return location_;
  }
  /**
   * Location data reference
   */
  data_staging::location& location_ref() {
    return location_;
  }
  /**
   * Absolute location data reference
   */
  data_staging::location& transitive_location_ref() {
    return transitive_location_;
  }
  /**
   * Absolute location data const reference
   */
  const data_staging::location& transitive_location_cref() {
    return transitive_location_;
  }
  /**
   * Movement data const reference
   */
  const movement& movement_cref() const {
    return movement_;
  }
  /**
   * Movement data reference
   */
  data_staging::movement& movement_ref() {
    return movement_;
  }
  /**
   * Generic data const reference
   */
  const generic& generic_cref() const {
    return generic_;
  }
  /**
   * Generic data reference
   */
  data_staging::generic& generic_ref() {
    return generic_;
  }

  /**
   * Styling data const reference
   */
  const styling& styling_cref() const {
    return styling_;
  }
  /**
   * Styling data reference
   */
  data_staging::styling& styling_ref() {
    return styling_;
  }
  /**
   * Behavior data const reference
   */
  const behavior& behavior_cref() const {
    return behavior_;
  }
  /**
   * Behavior data reference
   */
  data_staging::behavior& behavior_ref() {
    return behavior_;
  }
  /**
   * Toroidal data const reference
   */
  const data_staging::toroidal& toroidal_cref() {
    return toroidal_;
  }
  /**
   * Toroidal data reference
   */
  data_staging::toroidal& toroidal_ref() {
    return toroidal_;
  }
  /**
   * Properties data const reference
   */
  const properties& properties_cref() const {
    return properties_;
  }
  /**
   * Properties data reference
   */
  data_staging::properties& properties_ref() {
    return properties_;
  }

  /**
   * Longest diameter (A of the ellipse)
   * @return
   */
  double longest_diameter() {
    return longest_diameter_;
  }

  /**
   * Shortest diameter (B of the ellipse)
   * @return
   */
  double shortest_diameter() {
    return shortest_diameter_;
  }

  /**
   * Radius size
   */
  double radius_size() const {
    return radius_size_;
  }
  /**
   * Set longest diameter
   * @param longest_diameter
   */
  void set_longest_diameter(double longest_diameter) {
    longest_diameter_ = longest_diameter;
  }

  /**
   * Set shortest diameter
   * @param shortest_diameter
   */
  void set_shortest_diameter(double shortest_diameter) {
    shortest_diameter_ = shortest_diameter;
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
  const std::vector<cascade>& cascade_out_cref() const {
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
