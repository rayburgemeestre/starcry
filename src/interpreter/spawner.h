/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "data_staging/shape.hpp"
#include "util/v8_interact.hpp"

class generator_context_wrapper;

namespace interpreter {

class object_definitions;
class instantiator;
class object_lookup;
class scenes;

class spawner {
public:
  explicit spawner(generator_context_wrapper& genctx,
                   object_definitions& definitions,
                   instantiator& instantiator,
                   object_lookup& object_lookup,
                   scenes& scenes);

  int64_t spawn_object(data_staging::shape_t& spawner, v8::Local<v8::Object> obj);
  int64_t spawn_object2(data_staging::shape_t& spawner, v8::Local<v8::Object> line_obj, int64_t obj1);
  int64_t spawn_object3(data_staging::shape_t& spawner, v8::Local<v8::Object> line_obj, int64_t obj1, int64_t obj2);
  int64_t spawn_object_at_parent(data_staging::shape_t& spawner, v8::Local<v8::Object> obj);
  static int64_t destroy(data_staging::shape_t& spawner);

private:
  generator_context_wrapper& genctx;
  object_definitions& object_definitions_;
  instantiator& instantiator_;
  object_lookup& object_lookup_;
  scenes& scenes_;
};
}  // namespace interpreter
