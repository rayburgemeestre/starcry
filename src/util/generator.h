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
                                        std::unordered_map<std::string, data::gradient>& known_gradients_map,
                                        std::string* gradient_id_str = nullptr);

void copy_texture_from_object_to_shape(v8_interact& i,
                                       v8::Local<v8::Object>& source_object,
                                       data::shape& destination_shape,
                                       std::unordered_map<std::string, data::texture>& known_textures_map);

void instantiate_object(v8_interact& i,
                        std::optional<v8::Local<v8::Object>> scene_obj,
                        v8::Local<v8::Object> object_prototype,
                        v8::Local<v8::Object> new_instance,
                        int64_t level,
                        const std::string& namespace_);

v8::Local<v8::Object> instantiate_object_from_scene(v8_interact& i,
                                                    v8::Local<v8::Object>& objects,
                                                    v8::Local<v8::Array>& instances_next,
                                                    v8::Local<v8::Object>& scene_object,
                                                    v8::Local<v8::Object>* parent_object = nullptr);
void copy_instances(v8_interact& i, v8::Local<v8::Array> dest, v8::Local<v8::Array> source);

void garbage_collect_erased_objects(v8_interact& i,
                                    const v8::Local<v8::Array>& instances,
                                    const v8::Local<v8::Array>& intermediates,
                                    const v8::Local<v8::Array>& next_instances);

std::string instance_to_string(v8_interact& i, v8::Local<v8::Object>& instance);

void debug_print(v8_interact& i, v8::Local<v8::Object>& instance);
void debug_print(v8_interact& i,
                 v8::Local<v8::Array>& instances,
                 const std::string& desc,
                 int64_t unique_id_of_interest = -1);

v8::Local<v8::Object> get_object_by_id(v8_interact& i, v8::Local<v8::Array>& instances, int id);

}  // namespace generator
}  // namespace util