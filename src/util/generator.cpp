/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "util/generator.h"
#include "util/logger.h"

#include "signal.h"

#include <random>

namespace util {
namespace generator {

int64_t counter = 0;

void copy_gradient_from_object_to_shape(v8_interact& i,
                                        v8::Local<v8::Object>& source_object,
                                        data::shape& destination_shape,
                                        std::unordered_map<std::string, data::gradient>& known_gradients_map,
                                        std::string* gradient_id_str) {
  std::string gradient_id = i.str(source_object, "gradient");
  if (!gradient_id.empty()) {
    if (gradient_id_str) {
      *gradient_id_str += gradient_id;
    }
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
      if (gradient_id_str) {
        if (!gradient_id_str->empty()) *gradient_id_str += ",";
        *gradient_id_str += gradient_id;
      }
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
    i.copy_field_if_exists(new_instance, "radius", *scene_obj);
    i.copy_field_if_exists(new_instance, "radiussize", *scene_obj);
    i.copy_field_if_exists(new_instance, "gradient", *scene_obj);
    i.copy_field_if_exists(new_instance, "texture", *scene_obj);
    i.copy_field_if_exists(new_instance, "seed", *scene_obj);
    i.copy_field_if_exists(new_instance, "blending_type", *scene_obj);
    i.copy_field_if_exists(new_instance, "opacity", *scene_obj);
    i.copy_field_if_exists(new_instance, "scale", *scene_obj);
    i.copy_field_if_exists(new_instance, "angle", *scene_obj);
    i.copy_field_if_exists(new_instance, "pivot", *scene_obj);
    i.copy_field_if_exists(new_instance, "text", *scene_obj);
    i.copy_field_if_exists(new_instance, "text_align", *scene_obj);
    i.copy_field_if_exists(new_instance, "text_size", *scene_obj);
    i.copy_field_if_exists(new_instance, "text_fixed", *scene_obj);
  }

  i.set_field(new_instance, "subobj", v8::Array::New(isolate));
  i.set_field(new_instance, "level", v8::Number::New(isolate, level));
  {
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_int_distribution<int> distribution{'a', 'z'};
    static auto generate_len = 6;
    static std::string rand_str(generate_len, '\0');
    for (auto& dis : rand_str) dis = distribution(generator);
    i.set_field(new_instance, "__random_hash__", v8_str(context, rand_str));
  }
  i.set_field(new_instance, "__instance__", v8::Boolean::New(isolate, true));

  // Make sure we deep copy the gradients
  i.set_field(new_instance, "gradients", v8::Array::New(isolate));
  auto dest_gradients = i.get(new_instance, "gradients").As<v8::Array>();
  auto gradients = i.has_field(object_prototype, "gradients") && i.get(object_prototype, "gradients")->IsArray()
                       ? i.get(object_prototype, "gradients").As<v8::Array>()
                       : v8::Array::New(i.get_isolate());
  for (size_t k = 0; k < gradients->Length(); k++) {
    i.set_field(dest_gradients, k, v8::Array::New(isolate));

    auto gradient = i.get_index(gradients, k).As<v8::Array>();
    auto dest_gradient = i.get_index(dest_gradients, k).As<v8::Array>();
    for (size_t l = 0; l < gradient->Length(); l++) {
      i.set_field(dest_gradient, l, i.get_index(gradient, l));
    }
  }

  // Ensure we have a props object in the new obj
  if (!i.has_field(new_instance, "props")) {
    i.set_field(new_instance, "props", v8::Object::New(i.get_isolate()));
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

  auto the_fun = i.get_fun("__spawn__", new_instance);
  i.set_fun(new_instance, "spawn", the_fun);
}

v8::Local<v8::Object> instantiate_object_from_scene(
    v8_interact& i,
    v8::Local<v8::Array>& objects,         // the repository of objects
    v8::Local<v8::Array>& instances_dest,  // target instances
    v8::Local<v8::Object>& scene_object,   // object description from scene to be instantiated
    v8::Local<v8::Object>* parent_object   // it's optional parent
) {
  v8::Isolate* isolate = i.get_isolate();

  // determine level from it's parent
  int64_t current_level = (parent_object == nullptr) ? 0 : i.integer_number(*parent_object, "level") + 1;

  // lookup the object prototype to be instantiated
  auto object_id = i.str(scene_object, "id");
  auto object_prototype = v8_index_object(context, objects, object_id).template As<v8::Object>();

  // create a new javascript object
  v8::Local<v8::Object> instance = v8::Object::New(isolate);

  // instantiate the prototype into newly allocated javascript object
  instantiate_object(i, scene_object, object_prototype, instance, current_level);

  // give it a unique id (it already has been assigned a __random_hash__ for debugging purposes
  i.set_field(instance, "unique_id", v8::Number::New(i.get_isolate(), counter++));

  // push it into the destination array
  i.call_fun(instances_dest, "push", instance);

  // now invoke init() on the object, which in turn can lead to new objects
  i.call_fun(instance, "init");

  return instance;
}

void copy_instances(v8_interact& i, v8::Local<v8::Array> dest, v8::Local<v8::Array> source) {
  std::unordered_map<int64_t, size_t> obj_indexes;
  // first pass, mirror both top-level arrays

  for (size_t j = 0; j < source->Length(); j++) {
    // resize dest array if needed
    if (dest->Length() < source->Length()) {
      i.call_fun(dest, "push", v8::Object::New(i.get_isolate()));
    }
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
    i.copy_field(dst, "id", src);
    i.copy_field(dst, "label", src);
    i.copy_field(dst, "level", src);
    i.copy_field(dst, "unique_id", src);
    i.copy_field(dst, "type", src);
    i.copy_field(dst, "collision_group", src);
    i.copy_field_if_exists(dst, "blending_type", src);
    i.copy_field_if_exists(dst, "scale", src);
    i.copy_field_if_exists(dst, "opacity", src);
    i.copy_field_if_exists(dst, "angle", src);
    i.copy_field_if_exists(dst, "pivot", src);
    i.copy_field_if_exists(dst, "text", src);
    i.copy_field_if_exists(dst, "text_align", src);
    i.copy_field_if_exists(dst, "text_size", src);
    i.copy_field_if_exists(dst, "text_fixed", src);
    i.copy_field(dst, "__time__", src);
    i.copy_field(dst, "__elapsed__", src);
    i.copy_field_if_exists(dst, "__dist__", src);
    i.copy_field_if_exists(dst, "step", src);
    i.copy_field_if_exists(dst, "inherited", src);
    i.copy_field_if_exists(dst, "exists", src);
    // functions
    i.copy_field(dst, "on", src);
    i.copy_field(dst, "init", src);
    i.copy_field(dst, "init2", src);
    i.copy_field(dst, "time", src);
    // props
    i.set_field(dst, "props", v8::Object::New(i.get_isolate()));

    // subobj
    if (!i.has_field(dst, "subobj")) {
      i.set_field(dst, "subobj", v8::Array::New(i.get_isolate()));
    }
    if (i.has_field(src, "subobj")) {
      const auto s = i.get(src, "subobj").As<v8::Array>();
      const auto d = i.get(dst, "subobj").As<v8::Array>();
      for (size_t k = 0; k < s->Length(); k++) {
        if (d->Length() < s->Length()) {
          i.call_fun(d, "push", v8::Object::New(i.get_isolate()));
        }
        auto sub_src = i.get_index(s, k).As<v8::Object>();
        auto sub_dst = i.get_index(d, k).As<v8::Object>();
        i.recursively_copy_object(sub_dst, sub_src);
      }
      while (d->Length() > s->Length()) {
        i.call_fun(d, "pop");
      }
    }
    // gradient_s_
    if (!i.has_field(dst, "gradients")) {
      i.set_field(dst, "gradients", v8::Array::New(i.get_isolate()));
    }
    if (i.has_field(src, "gradients")) {
      const auto s = i.get(src, "gradients").As<v8::Array>();
      const auto d = i.get(dst, "gradients").As<v8::Array>();
      for (size_t k = 0; k < s->Length(); k++) {
        if (d->Length() < s->Length()) {
          i.call_fun(d, "push", v8::Object::New(i.get_isolate()));
        }
        const auto elem = i.get_index(s, k);
        i.set_field(d, k, elem);
      }
      while (d->Length() > s->Length()) {
        i.call_fun(d, "pop");
      }
    }

    // update object unique id to index mapping
    auto unique_id = i.integer_number(dst, "unique_id");
    obj_indexes[unique_id] = j;
  }

  // after first pass, if the dest array was too long, discard extra elements
  while (dest->Length() > source->Length()) {
    i.call_fun(dest, "pop");
  }

  // second pass over destination array
  for (size_t j = 0; j < dest->Length(); j++) {
    auto src = i.get_index(source, j).As<v8::Object>();
    auto dst = i.get_index(dest, j).As<v8::Object>();

    // fix subobjs
    const auto d = i.get(dst, "subobj").As<v8::Array>();
    for (size_t k = 0; k < d->Length(); k++) {
      auto sub_dst = i.get_index(d, k).As<v8::Object>();
      auto unique_id = i.integer_number(sub_dst, "unique_id");
      auto pos = obj_indexes.find(unique_id);
      if (pos == obj_indexes.end()) {
        logger(WARNING) << "Found a unique_id in subobj that shouldn't exist!" << std::endl;
        std::exit(1);
      }
      auto the_subobj = i.get_index(dest, pos->second).As<v8::Object>();
      i.set_field(d, k, the_subobj);
    }

    if (i.has_field(src, "props")) {
      auto d = i.get(dst, "props").As<v8::Object>();
      auto s = i.get(src, "props").As<v8::Object>();
      i.recursively_copy_object(d, s);
    }

    // fix props, we need to recursively walk it and make sure we fix all objects with a unique_id
    auto props = i.v8_obj(dst, "props");
    std::function<void(v8::Local<v8::Object> & obj)> walk =
        [&i, &obj_indexes, &dest, &walk](v8::Local<v8::Object>& obj) {
          auto obj_fields = i.prop_names(obj);
          for (size_t k = 0; k < obj_fields->Length(); k++) {
            auto field_name = i.get_index(obj_fields, k);
            auto field_value = i.get(obj, field_name);
            if (field_value->IsObject()) {
              auto field_value_obj = field_value.As<v8::Object>();
              if (i.has_field(field_value_obj, "unique_id")) {
                auto unique_id = i.integer_number(field_value_obj, "unique_id");
                auto pos = obj_indexes.find(unique_id);
                if (pos == obj_indexes.end()) {
                  logger(WARNING) << "Found a unique_id in subobj that shouldn't exist!" << std::endl;
                  std::exit(1);
                }
                auto the_subobj = i.get_index(dest, pos->second).As<v8::Object>();
                i.set_field(obj, field_name, the_subobj);  // now replaced with proper reference
              } else {
                // recurse on the object, it may contain references to instances
                walk(field_value_obj);
              }
            } else if (field_value->IsArray()) {
              // for arrays we have to check each element that is an object
              auto field_value_arr = field_value.As<v8::Array>();
              for (size_t l = 0; l < field_value_arr->Length(); l++) {
                auto array_elem = i.get_index(field_value_arr, l);
                if (array_elem->IsObject()) {
                  auto array_elem_obj = array_elem.As<v8::Object>();
                  walk(array_elem_obj);
                } else {
                  // leave array element alone, not a javascript object
                }
              }
            } else {
              // leave it alone, not a javascript object
            }
          }
        };
    walk(props);
  }
}

void garbage_collect_erased_objects(v8_interact& i, v8::Local<v8::Array>& instances) {
  size_t remove = 0;
  for (size_t j = 0; j < instances->Length(); j++) {
    auto instance = i.get_index(instances, j).As<v8::Object>();
    bool is_removed = i.has_field(instance, "exists") && !i.boolean(instance, "exists");
    if (is_removed) {
      // Remove this item (we'll overwrite or pop() it later)
      remove++;

      // Make all objects underneath it top level objects
      std::function<void(v8_interact&, v8::Local<v8::Object>&, int)> update_level =
          [&](v8_interact& i, v8::Local<v8::Object>& instance, int current_level = -1) {
            i.set_field(instance, "level", v8::Number::New(i.get_isolate(), current_level));
            auto instance_subobjs = i.get(instance, "subobj").As<v8::Array>();
            for (size_t k = 0; k < i.get(instance, "subobj").As<v8::Array>()->Length(); k++) {
              auto instance_subobj = i.get_index(instance_subobjs, k).As<v8::Object>();
              update_level(i, instance_subobj, current_level + 1);
            }
          };

      // TODO: also purge references such as left[] / right[] props.

      // The instances that has been removed will start the level at -1, making all it's direct children 0, children
      // below it 1, and so on..
      update_level(i, instance, -1);

    } else if (remove > 0) {
      // Keep this item
      i.set_field(instances, j - remove, instance);
    }
  }
  for (size_t j = 0; j < remove; j++) {
    i.call_fun(instances, "pop");
  }
}

std::string instance_to_string(v8_interact& i, v8::Local<v8::Object>& instance) {
  std::stringstream ss;
  if (!instance->IsObject() || !i.has_field(instance, "level")) {
    ss << "** UNKNOWN OBJECT **";
    if (instance->IsNull()) {
      ss << " (is null)";
    }
    if (instance->IsUndefined()) {
      ss << " (is undefined)";
    }
    return ss.str();
  }
  const auto level = i.integer_number(instance, "level");

  ss << "obj: " << i.integer_number(instance, "unique_id") << " " << i.str(instance, "id") << " "
     << i.str(instance, "type") << " " << level << " (" << i.double_number(instance, "x") << ", "
     << i.double_number(instance, "y") << ")";

  if (i.str(instance, "type") == "line") {
    ss << ",(" << i.double_number(instance, "x2") << ", " << i.double_number(instance, "y2") << ")";
  }
  ss << " " << i.str(instance, "label") << " [" << i.str(instance, "__random_hash__") << "] "
     << i.double_number(instance, "__dist__") << "/" << i.double_number(instance, "steps");

  if (i.has_field(instance, "velocity")) {
    ss << " (vel:" << i.double_number(instance, "velocity") << ")";
  }
  //  if (i.has_field(instance, "scale")) {
  //    ss << " (scale:" << i.double_number(instance, "scale") << ")";
  //  }

  return ss.str();
}

void debug_print(v8_interact& i, v8::Local<v8::Object>& instance) {
  const auto indent = [](int level) {
    std::stringstream ss;
    for (int i = 0; i < level; i++) {
      ss << "  ";
    }
    return ss.str();
  };
  const auto level = i.integer_number(instance, "level");
  logger(INFO) << indent(level) << instance_to_string(i, instance) << std::endl;
  if (i.has_field(instance, "subobj")) {
    auto subobjs = i.get(instance, "subobj").As<v8::Array>();
    for (size_t k = 0; k < subobjs->Length(); k++) {
      auto object = i.get_index(subobjs, k).As<v8::Object>();
      logger(INFO) << indent(level + 2) << "subobj: " << instance_to_string(i, object) << std::endl;
    }
  }
  if (i.has_field(instance, "props")) {
    auto props = i.v8_obj(instance, "props");
    auto obj_fields = i.prop_names(props);
    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_value = i.get(props, field_name);
      if (!field_value->IsObject()) continue;
      auto str = i.str(obj_fields, k);
      if (str == "left" || str == "right") {
        if (field_value->IsArray()) {
          auto a = field_value.As<v8::Array>();
          for (size_t l = 0; l < a->Length(); l++) {
            auto o = i.get_index(a, l).As<v8::Object>();
            logger(INFO) << indent(level + 2) << "prop." << str << ": " << instance_to_string(i, o) << std::endl;
          }
        } else if (field_value->IsObject()) {
          auto o = field_value.As<v8::Object>();
          logger(INFO) << indent(level + 2) << "prop!." << str << ": " << instance_to_string(i, o) << std::endl;
        }
      }
    }
  }
}

void debug_print(v8_interact& i, v8::Local<v8::Array>& instances, const std::string& desc) {
  logger(INFO) << "printing " << desc << ":" << std::endl;
  for (size_t j = 0; j < instances->Length(); j++) {
    auto instance = i.get_index(instances, j).As<v8::Object>();
    debug_print(i, instance);
  }
}

v8::Local<v8::Object> get_object_by_id(v8_interact& i, v8::Local<v8::Array>& instances, int id) {
  for (size_t j = 0; j < instances->Length(); j++) {
    auto instance = i.get_index(instances, j).As<v8::Object>();
    auto unique_id = i.integer_number(instance, "unique_id");
    if (unique_id == id) {
      return instance;
    }
  }
  return {};
}

}  // namespace generator
}  // namespace util
