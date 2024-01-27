/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "generator.h"

namespace interpreter {
object_lookup::object_lookup(generator& gen) : gen_(gen) {}

void object_lookup::update() {
  next_instance_map.clear();
  intermediate_map.clear();
  for (auto& abstract_shape : gen_.scenes_.next_shapes_current_scene()) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      next_instance_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
  for (auto& abstract_shape : gen_.scenes_.intermediate_shapes_current_scene()) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      intermediate_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
  mappings_dirty = false;
}

void object_lookup::reset() {
  next_instance_map.clear();
  intermediate_map.clear();
}

void object_lookup::update_if_dirty() {
  if (mappings_dirty) {
    update();
  }
}

void object_lookup::set_dirty() {
  mappings_dirty = true;
}

object_lookup::map_type::iterator object_lookup::find(int64_t id) {
  return next_instance_map.find(id);
}

object_lookup::map_type::iterator object_lookup::end() {
  return next_instance_map.end();
}

std::reference_wrapper<data_staging::shape_t>& object_lookup::at(int64_t id) {
  return next_instance_map.at(id);
}

bool object_lookup::contains(int64_t id) const {
  return next_instance_map.contains(id);
}

object_lookup::map_type::iterator object_lookup::find_intermediate(int64_t id) {
  return intermediate_map.find(id);
}

object_lookup::map_type::iterator object_lookup::end_intermediate() {
  return intermediate_map.end();
}

std::reference_wrapper<data_staging::shape_t>& object_lookup::at_intermediate(int64_t id) {
  return intermediate_map.at(id);
}

}  // namespace interpreter
