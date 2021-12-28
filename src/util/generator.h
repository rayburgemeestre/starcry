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
                        int64_t level);

std::tuple<v8::Local<v8::Object>, v8::Local<v8::Object>, v8::Local<v8::Object>> instantiate_objects(
    v8_interact& i,
    v8::Local<v8::Array>& objects,
    v8::Local<v8::Array>& scene_instances,
    v8::Local<v8::Array>& scene_instances_next,
    v8::Local<v8::Array>& scene_instances_intermediate,
    size_t& scene_instances_idx,
    v8::Local<v8::Object>& scene_object,
    v8::Local<v8::Object>* parent_object = nullptr);
void copy_instances(v8_interact& i, v8::Local<v8::Array> dest, v8::Local<v8::Array> source);

void garbage_collect_erased_objects(v8_interact& i,
                                    v8::Local<v8::Array>& scene_instances,
                                    v8::Local<v8::Array>& current_level,
                                    v8::Local<v8::Array>& scene_instances_intermediate);

void find_new_objects(v8_interact& i,
                      v8::Local<v8::Array>& objects,
                      v8::Local<v8::Array>& scene_instances,
                      v8::Local<v8::Array>& scene_instances_next,
                      v8::Local<v8::Array>& scene_instances_intermediate);

void monitor_subobj_changes(v8_interact& i, v8::Local<v8::Object> instance, std::function<void()> exec);

std::string instance_to_string(v8_interact& i, v8::Local<v8::Object>& instance);

void debug_print(v8_interact& i, v8::Local<v8::Object>& instance);
void debug_print(v8_interact& i, v8::Local<v8::Array>& instances, const std::string& desc);

v8::Local<v8::Object> get_object_by_id(v8_interact& i, v8::Local<v8::Array>& instances, int id);

}  // namespace generator
}  // namespace util