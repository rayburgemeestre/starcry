/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "util/generator.h"

namespace util {
namespace generator {
void copy_gradient_from_object_to_shape(v8_interact& i,
                                        v8::Local<v8::Object>& source_object,
                                        data::shape& destination_shape,
                                        std::unordered_map<std::string, data::gradient>& known_gradients_map) {
  std::string gradient_id = i.str(source_object, "gradient");
  if (!gradient_id.empty()) {
    if (destination_shape.gradients_.empty()) {
      if (known_gradients_map.find(gradient_id) != known_gradients_map.end()) {
        destination_shape.gradients_.emplace_back(1.0, known_gradients_map[gradient_id]);
      }
    }
  }
  auto gradient_array = i.v8_array(source_object, "gradients");
  if (destination_shape.gradients_.empty()) {
    for (size_t k = 0; k < gradient_array->Length(); k++) {
      auto gradient_data = i.get_index(gradient_array, k).As<v8::Array>();
      if (!gradient_data->IsArray()) {
        continue;
      }
      auto opacity = i.double_number(gradient_data, size_t(0));
      auto gradient_id = i.str(gradient_data, size_t(1));
      destination_shape.gradients_.emplace_back(opacity, known_gradients_map[gradient_id]);
    }
  }
}

void instantiate_object(v8_interact& i,
                        v8::Local<v8::Object> scene_obj,
                        v8::Local<v8::Object> object_prototype,
                        v8::Local<v8::Object> new_instance,
                        int64_t level,
                        const std::string& annotation) {
  v8::Isolate* isolate = i.get_isolate();

  i.recursively_copy_object(new_instance, object_prototype);
  // i.recursively_copy_object(new_instance, scene_obj);

  i.copy_field(new_instance, "id", scene_obj);
  i.copy_field(new_instance, "x", scene_obj);
  i.copy_field(new_instance, "y", scene_obj);
  i.copy_field(new_instance, "vel_x", scene_obj);
  i.copy_field(new_instance, "vel_y", scene_obj);

  i.set_field(new_instance, "subobj", v8::Array::New(isolate));
  i.set_field(new_instance, "meta", v8_str(context, annotation));
  i.set_field(new_instance, "level", v8::Number::New(isolate, level));
  i.copy_field(new_instance, "gradients", object_prototype);

  // Copy over scene properties to instance properties
  auto props = i.v8_obj(new_instance, "props");
  auto scene_props = i.v8_obj(scene_obj, "props");
  auto obj_fields = i.prop_names(scene_props);
  ;
  for (size_t k = 0; k < obj_fields->Length(); k++) {
    auto field_name = i.get_index(obj_fields, k);
    auto field_value = i.get(scene_props, field_name);
    i.set_field(props, field_name, field_value);
  }
  i.call_fun(new_instance, "init", 0.5);
}

v8::Local<v8::Object> instantiate_objects(v8_interact& i,
                                          v8::Local<v8::Array>& objects,
                                          v8::Local<v8::Array>& scene_instances,
                                          v8::Local<v8::Array>& scene_instances_next,
                                          v8::Local<v8::Array>& scene_instances_intermediate,
                                          size_t& scene_instances_idx,
                                          v8::Local<v8::Object>& scene_object,
                                          v8::Local<v8::Object>* parent_object) {
  v8::Isolate* isolate = i.get_isolate();
  int64_t current_level = (parent_object == nullptr) ? 0 : i.integer_number(*parent_object, "level") + 1;
  auto object_id = i.str(scene_object, "id");
  auto object_prototype = v8_index_object(context, objects, object_id).template As<v8::Object>();

  v8::Local<v8::Object> instance = v8::Object::New(isolate);
  v8::Local<v8::Object> next = v8::Object::New(isolate);
  v8::Local<v8::Object> intermediate = v8::Object::New(isolate);

  instantiate_object(i, scene_object, object_prototype, instance, current_level, "instance");
  instantiate_object(i, scene_object, object_prototype, next, current_level, "next");
  instantiate_object(i, scene_object, object_prototype, intermediate, current_level, "intermediate");

  i.set_field(scene_instances, scene_instances_idx, instance);
  i.set_field(scene_instances_next, scene_instances_idx, next);
  i.set_field(scene_instances_intermediate, scene_instances_idx, intermediate);
  scene_instances_idx++;

  // Recurse for the sub objects the init function populated.
  i.set_field(scene_object, "level", v8::Number::New(isolate, current_level));
  auto subobjs = i.v8_array(instance, "subobj");
  auto subobjs2 = i.v8_array(next, "subobj");
  auto subobjs3 = i.v8_array(intermediate, "subobj");
  for (size_t k = 0; k < subobjs->Length(); k++) {
    auto subobj = i.get_index(subobjs, k).As<v8::Object>();
    auto created_instance = instantiate_objects(i,
                                                objects,
                                                scene_instances,
                                                scene_instances_next,
                                                scene_instances_intermediate,
                                                scene_instances_idx,
                                                subobj,
                                                &scene_object);
    i.set_field(subobjs, k, created_instance);
    i.set_field(subobjs2, k, created_instance);
    i.set_field(subobjs3, k, created_instance);
  }
  return next;
}

void copy_instances(v8_interact& i, v8::Local<v8::Array> dest, v8::Local<v8::Array> source, bool exclude_props) {
  for (size_t j = 0; j < dest->Length(); j++) {
    auto src = i.get_index(source, j).As<v8::Object>();
    auto dst = i.get_index(dest, j).As<v8::Object>();
    i.copy_field(dst, "x", src);
    i.copy_field(dst, "y", src);
    i.copy_field(dst, "vel_x", src);
    i.copy_field(dst, "vel_y", src);
    i.copy_field(dst, "radius", src);
    i.copy_field(dst, "radiussize", src);
    i.copy_field(dst, "last_collide", src);
    if (!exclude_props) {
      i.copy_field(dst, "props", src);
    }
  }
}

}  // namespace generator
}  // namespace util
