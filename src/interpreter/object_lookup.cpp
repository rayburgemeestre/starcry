/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "generator.h"

namespace interpreter {
object_lookup::object_lookup(generator_context_wrapper& genctx, scenes& scenes) : genctx(genctx), scenes_(scenes) {}

void object_lookup::update() {
  next_instance_map.clear();
  intermediate_map.clear();
  for (auto& abstract_shape : scenes_.next_shapes_current_scene()) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      next_instance_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
  for (auto& abstract_shape : scenes_.intermediate_shapes_current_scene()) {
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

v8::Local<v8::Object> object_lookup::get_object(int64_t object_unique_id) {
  auto& i = genctx.get()->i();
  // BEGIN: Temporary code (to try out something)
  data_staging::shape_t* obj1o = nullptr;
  auto find1 = find(object_unique_id);
  if (find1 != end()) {
    obj1o = &find1->second.get();
  } else {
    // todo; we need a mapping for this...
    for (auto& newo : scenes_.instantiated_objects_current_scene()) {
      if (auto c = std::get_if<data_staging::circle>(&newo)) {
        if (c->meta_cref().unique_id() == object_unique_id) {
          obj1o = &newo;
        }
      }
    }
  }
  auto obj = v8::Object::New(i.get_isolate());

  meta_callback(*obj1o, [&]<typename T>(const T& cc) {
    i.set_field(obj, "unique_id", v8::Number::New(i.get_isolate(), cc.meta_cref().unique_id()));
    i.set_field(obj, "id", v8_str(i.get_context(), cc.meta_cref().id()));
  });
  return obj;
}

}  // namespace interpreter
