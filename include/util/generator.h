/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "data/shape.hpp"
#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"

namespace util {
namespace generator {

void copy_gradient_from_object_to_shape(v8_interact& i,
                                        v8::Local<v8::Object>& source_object,
                                        data::shape& destination_shape,
                                        std::unordered_map<std::string, data::gradient>& known_gradients_map);

void copy_texture_from_object_to_shape(v8_interact& i,
                                       v8::Local<v8::Object>& source_object,
                                       data::shape& destination_shape,
                                       std::unordered_map<std::string, data::texture>& known_textures_map);

void instantiate_object(v8_interact& i,
                        v8::Local<v8::Object> scene_obj,
                        v8::Local<v8::Object> object_prototype,
                        v8::Local<v8::Object> new_instance,
                        int64_t level,
                        const std::string& annotation);

v8::Local<v8::Object> instantiate_objects(v8_interact& i,
                                          v8::Local<v8::Array>& objects,
                                          v8::Local<v8::Array>& scene_instances,
                                          v8::Local<v8::Array>& scene_instances_next,
                                          v8::Local<v8::Array>& scene_instances_intermediate,
                                          size_t& scene_instances_idx,
                                          v8::Local<v8::Object>& scene_object,
                                          v8::Local<v8::Object>* parent_object = nullptr);
void copy_instances(v8_interact& i, v8::Local<v8::Array> dest, v8::Local<v8::Array> source, bool exclude_props = false);

}  // namespace generator
}  // namespace util