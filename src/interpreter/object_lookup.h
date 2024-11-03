/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdint>
#include <unordered_map>
#include "data_staging/shape.hpp"

class generator_context;

namespace interpreter {
class scenes;

class object_lookup {
public:
  using map_type = std::unordered_map<int64_t, std::reference_wrapper<data_staging::shape_t>>;

  explicit object_lookup(std::shared_ptr<generator_context>& genctx, scenes& scenes);

  void update();
  void reset();
  void update_if_dirty();
  void set_dirty();

  map_type::iterator find(int64_t id);
  map_type::iterator end();
  std::reference_wrapper<data_staging::shape_t>& at(int64_t id);
  bool contains(int64_t id) const;

  map_type::iterator find_intermediate(int64_t id);
  map_type::iterator end_intermediate();
  std::reference_wrapper<data_staging::shape_t>& at_intermediate(int64_t id);

  v8::Local<v8::Object> get_object(int64_t object_unique_id);

private:
  std::shared_ptr<generator_context>& genctx;
  scenes& scenes_;

  map_type next_instance_map;
  map_type intermediate_map;
  bool mappings_dirty = false;
};

}  // namespace interpreter