/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "util/generator.h"

namespace util {
namespace generator {

static int64_t counter = 0;

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

// TODO: this is almost a copy of the above copy_gradient_from_object_to_shape function
void copy_texture_from_object_to_shape(v8_interact& i,
                                       v8::Local<v8::Object>& source_object,
                                       data::shape& destination_shape,
                                       std::unordered_map<std::string, data::texture>& known_textures_map) {
  std::string texture_id = i.str(source_object, "texture");
  if (!texture_id.empty()) {
    if (destination_shape.textures.empty()) {
      if (known_textures_map.find(texture_id) != known_textures_map.end()) {
        destination_shape.textures.emplace_back(1.0, known_textures_map[texture_id]);
      }
    }
  }
  auto texture_array = i.v8_array(source_object, "textures");
  if (destination_shape.textures.empty()) {
    for (size_t k = 0; k < texture_array->Length(); k++) {
      auto texture_data = i.get_index(texture_array, k).As<v8::Array>();
      if (!texture_data->IsArray()) {
        continue;
      }
      auto opacity = i.double_number(texture_data, size_t(0));
      auto texture_id = i.str(texture_data, size_t(1));
      destination_shape.textures.emplace_back(opacity, known_textures_map[texture_id]);
    }
  }
}

void instantiate_object(v8_interact& i,
                        std::optional<v8::Local<v8::Object>> scene_obj,
                        v8::Local<v8::Object> object_prototype,
                        v8::Local<v8::Object> new_instance,
                        int64_t level) {
  v8::Isolate* isolate = i.get_isolate();

  i.recursively_copy_object(new_instance, object_prototype);
  // i.recursively_copy_object(new_instance, scene_obj);

  if (scene_obj) {
    i.copy_field(new_instance, "id", *scene_obj);
    if (i.has_field(*scene_obj, "label")) {
      i.copy_field(new_instance, "label", *scene_obj);
    }
    i.copy_field(new_instance, "x", *scene_obj);
    i.copy_field(new_instance, "y", *scene_obj);
    i.copy_field(new_instance, "x2", *scene_obj);
    i.copy_field(new_instance, "y2", *scene_obj);
    i.copy_field(new_instance, "vel_x", *scene_obj);
    i.copy_field(new_instance, "vel_y", *scene_obj);
    i.copy_field(new_instance, "vel_x2", *scene_obj);
    i.copy_field(new_instance, "vel_y2", *scene_obj);
    i.copy_field(new_instance, "velocity", *scene_obj);
    if (i.has_field(*scene_obj, "radius")) {
      i.copy_field(new_instance, "radius", *scene_obj);
    }
    if (i.has_field(*scene_obj, "radiussize")) {
      i.copy_field(new_instance, "radiussize", *scene_obj);
    }
    if (i.has_field(*scene_obj, "gradient")) {
      i.copy_field(new_instance, "gradient", *scene_obj);
    }
    if (i.has_field(*scene_obj, "texture")) {
      i.copy_field(new_instance, "texture", *scene_obj);
    }
    if (i.has_field(*scene_obj, "seed")) {
      i.copy_field(new_instance, "seed", *scene_obj);
    }
    if (i.has_field(*scene_obj, "blending_type")) {
      i.copy_field(new_instance, "blending_type", *scene_obj);
    }
    if (i.has_field(*scene_obj, "scale")) {
      i.copy_field(new_instance, "scale", *scene_obj);
    }
  }

  i.set_field(new_instance, "subobj", v8::Array::New(isolate));
  i.set_field(new_instance, "level", v8::Number::New(isolate, level));
  i.set_field(new_instance, "__instance__", v8::Boolean::New(isolate, true));

  // Make sure we deep copy the gradients
  i.set_field(new_instance, "gradients", v8::Array::New(isolate));
  auto dest_gradients = i.get(new_instance, "gradients").As<v8::Array>();
  auto gradients = i.get(object_prototype, "gradients").As<v8::Array>();
  for (size_t k = 0; k < gradients->Length(); k++) {
    i.set_field(dest_gradients, k, v8::Array::New(isolate));

    auto gradient = i.get_index(gradients, k).As<v8::Array>();
    auto dest_gradient = i.get_index(dest_gradients, k).As<v8::Array>();
    for (size_t l = 0; l < gradient->Length(); l++) {
      i.set_field(dest_gradient, l, i.get_index(gradient, l));
    }
  }

  // Copy over scene properties to instance properties
  if (scene_obj) {
    auto props = i.v8_obj(new_instance, "props");
    auto scene_props = i.v8_obj(*scene_obj, "props");
    auto obj_fields = i.prop_names(scene_props);

    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_value = i.get(scene_props, field_name);
      i.set_field(props, field_name, field_value);
    }
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

  instantiate_object(i, scene_object, object_prototype, instance, current_level);
  instantiate_object(i, scene_object, object_prototype, next, current_level);
  instantiate_object(i, scene_object, object_prototype, intermediate, current_level);

  i.set_field(instance, "unique_id", v8::Number::New(i.get_isolate(), counter));
  i.set_field(next, "unique_id", v8::Number::New(i.get_isolate(), counter));
  i.set_field(intermediate, "unique_id", v8::Number::New(i.get_isolate(), counter));
  counter++;

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
  for (size_t j = 0; j < source->Length(); j++) {
    auto src = i.get_index(source, j).As<v8::Object>();
    auto dst = i.get_index(dest, j).As<v8::Object>();
    i.copy_field(dst, "x", src);
    i.copy_field(dst, "y", src);
    i.copy_field(dst, "x2", src);
    i.copy_field(dst, "y2", src);
    i.copy_field(dst, "vel_x", src);
    i.copy_field(dst, "vel_y", src);
    i.copy_field(dst, "vel_x2", src);
    i.copy_field(dst, "vel_y2", src);
    i.copy_field(dst, "velocity", src);
    i.copy_field(dst, "radius", src);
    i.copy_field(dst, "radiussize", src);
    i.copy_field(dst, "last_collide", src);
    i.copy_field(dst, "gradient", src);
    i.copy_field(dst, "texture", src);
    i.copy_field(dst, "seed", src);
    // TODO: move has_field check into copy_field
    if (i.has_field(src, "blending_type")) {
      i.copy_field(dst, "blending_type", src);
    }
    // TODO: move has_field check into copy_field
    if (i.has_field(src, "scale")) {
      i.copy_field(dst, "scale", src);
    }
    i.copy_field(dst, "__time__", src);
    i.copy_field(dst, "__elapsed__", src);
    if (!exclude_props) {
      i.copy_field(dst, "props", src);
    }
    // TODO: move has_field check into copy_field
    if (i.has_field(src, "new_objects")) {
      i.copy_field(dst, "new_objects", src);
    }
    if (i.has_field(src, "subobj")) {
      i.copy_field(dst, "subobj", src);
    }
    if (i.has_field(src, "exists")) {
      i.copy_field(dst, "exists", src);
    }
  }
}

void find_new_objects(v8_interact& i,
                      v8::Local<v8::Array>& objects,
                      v8::Local<v8::Array>& scene_instances,
                      v8::Local<v8::Array>& scene_instances_next,
                      v8::Local<v8::Array>& scene_instances_intermediate) {
  v8::Isolate* isolate = i.get_isolate();
  std::unordered_map<size_t, std::vector<std::array<v8::Local<v8::Object>, 3>>> new_instances;
  size_t total_new_instances = 0;

  // go over all instances and see if any of them added new objects for instantiation
  for (size_t j = 0; j < scene_instances_next->Length(); j++) {
    auto next = i.get_index(scene_instances_next, j).As<v8::Object>();
    auto instance = i.get_index(scene_instances, j).As<v8::Object>();
    auto intermediate = i.get_index(scene_instances_intermediate, j).As<v8::Object>();
    if (i.has_field(next, "new_objects") && i.boolean(next, "new_objects")) {
      if (i.has_field(next, "subobj")) {
        auto subobjs = i.get(next, "subobj").As<v8::Array>();
        auto subobjs2 = i.get(instance, "subobj").As<v8::Array>();
        auto subobjs3 = i.get(intermediate, "subobj").As<v8::Array>();
        for (size_t k = 0; k < subobjs->Length(); k++) {
          auto object_definition = i.get_index(subobjs, k).As<v8::Object>();
          if (i.has_field(object_definition, "__instance__")) {
            // these are already instantiated, ignore these
            continue;
          }
          auto object_id = i.str(object_definition, "id");
          auto object_prototype = v8_index_object(context, objects, object_id).template As<v8::Object>();

          v8::Local<v8::Object> new_instance = v8::Object::New(isolate);
          v8::Local<v8::Object> new_next = v8::Object::New(isolate);
          v8::Local<v8::Object> new_intermediate = v8::Object::New(isolate);
          int64_t current_level = i.integer_number(next, "level") + 1;

          instantiate_object(i, object_definition, object_prototype, new_instance, current_level);
          instantiate_object(i, object_definition, object_prototype, new_next, current_level);
          instantiate_object(i, object_definition, object_prototype, new_intermediate, current_level);

          i.set_field(new_instance, "unique_id", v8::Number::New(i.get_isolate(), counter));
          i.set_field(new_next, "unique_id", v8::Number::New(i.get_isolate(), counter));
          i.set_field(new_intermediate, "unique_id", v8::Number::New(i.get_isolate(), counter));
          counter++;

          i.set_field(new_instance, "exists", v8::Boolean::New(i.get_isolate(), false));
          i.set_field(new_next, "exists", v8::Boolean::New(i.get_isolate(), true));
          i.set_field(new_intermediate, "exists", v8::Boolean::New(i.get_isolate(), true));

          // we'll need to insert the new instances at the correct position in the arrays
          // we store the index where they have to be inserted, which should be one past their
          // parent instance (hence j + 1)
          total_new_instances++;
          new_instances[j + total_new_instances].push_back({new_instance, new_next, new_intermediate});

          // make sure the subobj will also point to the new instance
          i.set_field(subobjs, k, new_next);
          i.set_field(subobjs2, k, new_instance);
          i.set_field(subobjs3, k, new_intermediate);
        }
      }
      i.set_field(next, "new_objects", v8::Boolean::New(i.get_isolate(), false));
      i.set_field(intermediate, "new_objects", v8::Boolean::New(i.get_isolate(), false));
      i.set_field(instance, "new_objects", v8::Boolean::New(i.get_isolate(), false));
    }
  }

  if (total_new_instances == 0) {
    return;
  }
  // insert all the newly created instances in the appropriate arrays
  // step 1: make sure we add space in the arrays (or we'll write out of bounds)
  for (size_t j = 0; j < total_new_instances; j++) {
    // push is highly optimized
    i.call_fun(scene_instances, "push", v8::Object::New(isolate));
    i.call_fun(scene_instances_next, "push", v8::Object::New(isolate));
    i.call_fun(scene_instances_intermediate, "push", v8::Object::New(isolate));
  }
  // step 2: walk backwards over the arrays, and move elements to the end, inserting where needed.
  auto items_left = total_new_instances;
  for (size_t j = scene_instances_next->Length() - 1; j != std::numeric_limits<size_t>::max() && items_left > 0; j--) {
    auto from = j - items_left;
    auto to = j;
    if (new_instances.find(to) == new_instances.end()) {
      // no need to insert, simply move the element
      i.set_field(scene_instances, to, i.get_index(scene_instances, from).As<v8::Object>());
      i.set_field(scene_instances_next, to, i.get_index(scene_instances_next, from).As<v8::Object>());
      i.set_field(scene_instances_intermediate, to, i.get_index(scene_instances_intermediate, from).As<v8::Object>());
    } else {
      // insert elements for this index
      for (const auto& new_instance : new_instances[j]) {
        i.set_field(scene_instances, to, new_instance[0]);
        i.set_field(scene_instances_next, to, new_instance[1]);
        i.set_field(scene_instances_intermediate, to, new_instance[2]);
        items_left--;
        to--;
      }
      j -= new_instances[j].size() - 1;
    }
  }
}

}  // namespace generator
}  // namespace util
