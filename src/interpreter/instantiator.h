/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "data_staging/shape.hpp"
#include "util/random.hpp"
#include "util/v8_interact.hpp"

namespace interpreter {

void instantiate_object_copy_fields(v8_interact& i,
                                    v8::Local<v8::Object> scene_obj,
                                    v8::Local<v8::Object> new_instance);

class generator;
class object_definitions;

class instantiator {
private:
public:
  explicit instantiator(generator& gen, object_definitions& definitions);

  void instantiate_additional_objects_from_new_scene(v8::Persistent<v8::Array>& scene_objects,
                                                     int debug_level = 0,
                                                     const data_staging::shape_t* parent_object = nullptr);

  std::optional<std::tuple<v8::Local<v8::Object>, std::reference_wrapper<data_staging::shape_t>, data_staging::shape_t>>
  instantiate_object_from_scene(
      v8_interact& i,
      v8::Local<v8::Object>& scene_object,          // object description from scene to be instantiated
      const data_staging::shape_t* parent_object);  // it's optional parent

  void reset_seeds();

private:
  void _instantiate_object(v8_interact& i,
                           std::optional<v8::Local<v8::Object>> scene_obj,
                           v8::Local<v8::Object> object_prototype,
                           v8::Local<v8::Object> new_instance,
                           int64_t level,
                           const std::string& namespace_);

  template <typename T>
  void write_back_copy(T& copy);

  generator& gen_;
  object_definitions& definitions_;
  util::random_generator rand_;
  int64_t counter = 0;
};
}  // namespace interpreter
