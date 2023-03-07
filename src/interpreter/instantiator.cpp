/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "instantiator.h"
#include "generator.h"

namespace interpreter {

void instantiate_object_copy_fields(v8_interact& i,
                                    v8::Local<v8::Object> scene_obj,
                                    v8::Local<v8::Object> new_instance) {
  i.copy_field_if_exists(new_instance, "id", scene_obj);
  i.copy_field_if_exists(new_instance, "x", scene_obj);
  i.copy_field_if_exists(new_instance, "y", scene_obj);
  i.copy_field_if_exists(new_instance, "x2", scene_obj);
  i.copy_field_if_exists(new_instance, "y2", scene_obj);
  i.copy_field_if_exists(new_instance, "vel_x", scene_obj);
  i.copy_field_if_exists(new_instance, "vel_y", scene_obj);
  i.copy_field_if_exists(new_instance, "vel_x2", scene_obj);
  i.copy_field_if_exists(new_instance, "vel_y2", scene_obj);
  i.copy_field_if_exists(new_instance, "velocity", scene_obj);
  i.copy_field_if_exists(new_instance, "mass", scene_obj);
  i.copy_field_if_exists(new_instance, "radius", scene_obj);
  i.copy_field_if_exists(new_instance, "radiussize", scene_obj);
  i.copy_field_if_exists(new_instance, "gradient", scene_obj);
  i.copy_field_if_exists(new_instance, "texture", scene_obj);
  i.copy_field_if_exists(new_instance, "seed", scene_obj);
  i.copy_field_if_exists(new_instance, "blending_type", scene_obj);
  i.copy_field_if_exists(new_instance, "opacity", scene_obj);
  i.copy_field_if_exists(new_instance, "scale", scene_obj);
  i.copy_field_if_exists(new_instance, "angle", scene_obj);
  i.copy_field_if_exists(new_instance, "rotate", scene_obj);
  i.copy_field_if_exists(new_instance, "hue", scene_obj);
  i.copy_field_if_exists(new_instance, "pivot", scene_obj);
  i.copy_field_if_exists(new_instance, "text", scene_obj);
  i.copy_field_if_exists(new_instance, "text_align", scene_obj);
  i.copy_field_if_exists(new_instance, "text_size", scene_obj);
  i.copy_field_if_exists(new_instance, "text_fixed", scene_obj);
  i.copy_field_if_exists(new_instance, "text_font", scene_obj);
  // this function is also used for parent -> child field inheritence.
  // for scripts, the 'file' field should never be inherited.
  // i.copy_field_if_exists(new_instance, "file", scene_obj);
  i.copy_field_if_exists(new_instance, "duration", scene_obj);
  i.copy_field_if_exists(new_instance, "collision_group", scene_obj);
  i.copy_field_if_exists(new_instance, "gravity_group", scene_obj);
  i.copy_field_if_exists(new_instance, "unique_group", scene_obj);
}

void recursively_build_stack_for_object(auto& new_stack,
                                        auto& shape,
                                        auto& next_instance_map,
                                        std::vector<data_staging::shape_t>& instantiated_objs,
                                        int level = 0) {
  meta_callback(shape, [&](const auto& cc) {
    new_stack.emplace_back(shape);
    if (cc.meta_cref().level() > 0) {
      if (next_instance_map.find(cc.meta_cref().parent_uid()) != next_instance_map.end()) {
        recursively_build_stack_for_object(new_stack,
                                           next_instance_map.at(cc.meta_cref().parent_uid()).get(),
                                           next_instance_map,
                                           instantiated_objs,
                                           level + 1);
      } else {
        for (auto& v : instantiated_objs) {
          meta_callback(v, [&](const auto& ccc) {
            if (ccc.meta_cref().unique_id() == cc.meta_cref().parent_uid()) {
              recursively_build_stack_for_object(new_stack, v, next_instance_map, instantiated_objs, level + 1);
            }
          });
        }
      }
    }
  });
}

instantiator::instantiator(generator& gen) : gen_(gen) {}

void instantiator::instantiate_additional_objects_from_new_scene(v8::Persistent<v8::Array>& scene_objects,
                                                                 int debug_level,
                                                                 const data_staging::shape_t* parent_object) {
  auto& i = gen_.genctx->i();
  std::string namespace_;

  if (parent_object)
    meta_callback(*parent_object, [&]<typename T>(const T& shape) {
      namespace_ = shape.meta_cref().namespace_name();
      logger(DEBUG) << std::string(debug_level, ' ')
                    << "Instantiating additional objects from new scene for parent object: " << shape.meta_cref().id()
                    << std::endl;
    });
  else {
    logger(DEBUG) << std::string(debug_level, ' ')
                  << "Instantiating additional objects from new scene for parent object NULL" << std::endl;
  }

  // instantiate all the additional objects from the new scene
  for (size_t j = 0; j < scene_objects.Get(i.get_isolate())->Length(); j++) {
    auto scene_obj = i.get_index(scene_objects, j).As<v8::Object>();
    auto scene_obj_id = i.str(scene_obj, "id");
    logger(DEBUG) << std::string(debug_level, ' ') << "Instantiating object id: " << scene_obj_id
                  << " (namespace: " << namespace_ << ")" << std::endl;

    /**
     * when specifying 'scene_inheritance_has_priority' to 'true', that means that we won't
     * ever overwrite certain fields, such as 'gradient' or 'unique_group' to spawned objects
     */
    auto instantiated_object = instantiate_object_from_scene(i, scene_obj, parent_object, true);
    if (instantiated_object) {
      auto [created_instance, shape_ref, shape_copy] = *instantiated_object;
      gen_.create_bookkeeping_for_script_objects(created_instance, shape_copy, debug_level + 1);
    }
  }
}

std::optional<std::tuple<v8::Local<v8::Object>, std::reference_wrapper<data_staging::shape_t>, data_staging::shape_t>>
instantiator::instantiate_object_from_scene(
    v8_interact& i,
    v8::Local<v8::Object>& scene_object,         // object description from scene to be instantiated
    const data_staging::shape_t* parent_object,  // it's optional parent
    bool scene_inheritance_has_priority) {
  v8::Isolate* isolate = i.get_isolate();

  int64_t current_level = 0;
  auto parent_object_ns = std::string("");
  int64_t parent_uid = -1;

  if (parent_object) {
    meta_callback(*parent_object, [&]<typename T>(const T& cc) {
      current_level = cc.meta_cref().level() + 1;
      parent_object_ns = cc.meta_cref().namespace_name();
      parent_uid = cc.meta_cref().unique_id();
    });
  }

  // lookup the object prototype to be instantiated
  auto object_id = i.str(scene_object, "id", "");
  if (object_id != "__point__") {
    object_id = parent_object_ns + object_id;
  }
  auto object_prototype = v8_index_object(i.get_context(), gen_.genctx->objects, object_id).template As<v8::Object>();

  // logger(DEBUG) << "instantiate_object_from_scene, prototype: " << object_id << std::endl;

  // create a new javascript object
  v8::Local<v8::Object> instance = v8::Object::New(isolate);

  // TODO: make sure this is the only source..., and get rid of genctx->objects usage
  if (!object_prototype->IsObject()) {
    object_prototype = gen_.object_definitions_map[object_id].Get(isolate);
  }

  if (!object_prototype->IsObject()) {
    logger(WARNING) << "cannot instantiate object id: " << object_id << ", does not exist" << std::endl;
    throw std::runtime_error(fmt::format("cannot instantiate object id: {}, does not exist", object_id));
  }

  // instantiate the prototype into newly allocated javascript object
  _instantiate_object(i, scene_object, object_prototype, instance, current_level, parent_object_ns);

  // inherit some fields from parent
  //  if (parent_object) {
  //    meta_callback(const_cast<data_staging::shape_t&>(*parent_object), [&](auto& shape) {
  //      if (!scene_inheritance_has_priority) {
  //          // dealing with a parent spawning a child, which should inherit explicitly specified fields only
  //        if (shape.styling_cref().gradient().size())
  //          i.set_field(instance, "gradient", v8_str(i.get_context(), shape.styling_ref().gradient()));
  //        if (shape.behavior_cref().unique_group_ref().size())
  //          i.set_field(instance, "unique_group", v8_str(i.get_context(), shape.behavior_ref().unique_group()));
  //      }
  //    });
  //  }

  // give it a unique id (it already has been assigned a __random_hash__ for debugging purposes
  static int64_t counter = 0;
  i.set_field(instance, "unique_id", v8::Number::New(i.get_isolate(), ++counter));
  i.set_field(instance, "parent_uid", v8::Number::New(i.get_isolate(), parent_uid));

  // TODO: in the future we will simply instantiate this directly, for now, to save some refactoring time
  // we will map, to see if the proof of concept works

  // TODO: try to use a raw pointer, see if it improves performance
  std::optional<std::reference_wrapper<data_staging::shape_t>> shape_ref;
  std::optional<data_staging::shape_t> shape_copy;

  const auto handle = [&]<typename T>(T& c) -> data_staging::shape_t& {
    // we buffer instantiated objects, and will insert in the array later.
    gen_.scenes_.instantiated_objects_current_scene().emplace_back(c);
    return gen_.scenes_.instantiated_objects_current_scene().back();
  };

  const auto type = i.str(gen_.object_definitions_map[object_id], "type", "");
  bool check_uniqueness = false;
  std::string unique_group = "";

  const auto initialize = [&]<typename T>(T& c, auto& bridge) {
    c.meta_ref().set_level(current_level);
    c.meta_ref().set_parent_uid(parent_uid);
    c.meta_ref().set_pivot(i.boolean(instance, "pivot"));
    c.behavior_ref().set_collision_group(i.str(instance, "collision_group", ""));
    c.behavior_ref().set_gravity_group(i.str(instance, "gravity_group", ""));
    c.behavior_ref().set_unique_group(i.str(instance, "unique_group", ""));
    check_uniqueness = c.behavior_ref().unique_group_ref().size();
    unique_group = c.behavior_ref().unique_group_ref();
    c.toroidal_ref().set_group(i.str(instance, "toroidal", ""));
    c.generic_ref().set_angle(i.double_number(instance, "angle", 0));
    c.generic_ref().set_rotate(i.double_number(instance, "rotate", 0));
    c.styling_ref().set_hue(i.double_number(instance, "hue", 0));
    c.generic_ref().set_opacity(i.double_number(instance, "opacity", 1));
    c.generic_ref().set_mass(i.double_number(instance, "mass", 1));
    c.generic_ref().set_scale(i.double_number(instance, "scale", 1));
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      if (i.has_field(instance, "texture")) {
        c.styling_ref().set_texture(i.str(instance, "texture"));
      }
      if (i.has_field(instance, "gradient")) {
        c.styling_ref().set_gradient(i.str(instance, "gradient"));
      }
    }
    if (i.has_field(instance, "gradients")) {
      auto gradient_array = i.v8_array(instance, "gradients");
      for (size_t k = 0; k < gradient_array->Length(); k++) {
        auto gradient_data = i.get_index(gradient_array, k).As<v8::Array>();
        if (!gradient_data->IsArray()) continue;
        auto opacity = i.double_number(gradient_data, size_t(0));
        auto gradient_id = parent_object_ns + i.str(gradient_data, size_t(1));
        if constexpr (!std::is_same_v<T, data_staging::script>) {
          c.styling_ref().add_gradient(opacity, gradient_id);
        }
      }
    }
    if (i.has_field(instance, "props")) {
      auto props_object = i.v8_obj(instance, "props");
      auto obj_fields = i.prop_names(props_object);
      for (size_t k = 0; k < obj_fields->Length(); k++) {
        auto field_name = i.get_index(obj_fields, k);
        auto field_name_str = i.str(obj_fields, k);
        auto field_value = i.get(props_object, field_name);
        i.set_field(c.properties_ref().properties_ref(), field_name, field_value);
      }
    }

    // the handle function returns a ref, which is all fine, but recursively init
    // may actually invalidate this ref with other inserts.
    shape_ref = std::ref(handle(c));
    shape_copy = (*shape_ref).get();

    // call init last, so that objects exist when using 'this' inside init()
    if (bridge) {
      // take a copy as the reference might point to a non-existant instance at some point,
      // for example when other cascading 'init's insert new objects.
      auto copy = std::get<T>((*shape_ref).get());
      bridge->push_object(copy);

      i.call_fun(gen_.object_definitions_map[object_id],  // object definition
                 bridge->instance(),                      // bridged object is "this"
                 "init");
      bridge->pop_object();
      write_back_copy(copy);
    }
  };

  if (type == "circle" || type.empty() /* treat every "non-type" as circles too */) {
    data_staging::circle c(object_id,
                           counter,
                           vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                           i.double_number(instance, "radius"),
                           i.double_number(instance, "radiussize"));

    if (type.empty()) c.generic_ref().set_opacity(0);

    c.movement_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                  i.double_number(instance, "vel_y", 0),
                                  i.double_number(instance, "velocity", 0));

    c.meta_ref().set_namespace(parent_object_ns);

    c.styling_ref().set_seed(i.integer_number(instance, "seed", 0));
    c.styling_ref().set_blending_type(i.integer_number(instance, "blending_type"));

    initialize(c, gen_.bridges_.circle());

  } else if (type == "line") {
    data_staging::line l(object_id,
                         counter,
                         vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                         vector2d(i.double_number(instance, "x2"), i.double_number(instance, "y2")),
                         i.double_number(instance, "radiussize"));

    // TODO: no logic for end of line
    l.movement_line_start_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                             i.double_number(instance, "vel_y", 0),
                                             i.double_number(instance, "velocity", 0));

    l.meta_ref().set_namespace(parent_object_ns);

    l.styling_ref().set_blending_type(i.integer_number(instance, "blending_type"));

    initialize(l, gen_.bridges_.line());
  } else if (type == "text") {
    data_staging::text t(object_id,
                         counter,
                         vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                         i.str(instance, "text"),
                         i.double_number(instance, "text_size"),
                         i.str(instance, "text_align"),
                         i.boolean(instance, "text_fixed"));

    t.set_font_name(i.has_field(instance, "text_font") ? i.str(instance, "text_font") : "monaco.ttf");

    t.movement_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                  i.double_number(instance, "vel_y", 0),
                                  i.double_number(instance, "velocity", 0));

    t.meta_ref().set_namespace(parent_object_ns);

    t.styling_ref().set_blending_type(i.integer_number(instance, "blending_type"));

    initialize(t, gen_.bridges_.text());

  } else if (type == "script") {
    data_staging::script s(
        object_id, counter, vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")));

    s.meta_ref().set_namespace(object_id + "_");

    initialize(s, gen_.bridges_.script());
  } else {
    throw std::logic_error(fmt::format("unknown type: {}", type));
  }

  if (!shape_ref) {
    throw std::runtime_error("unexpected shape_ref not set to reference");
  }

  if (check_uniqueness) {
    auto& created_shape_copy = *shape_copy;

    // we cannot trust shape_ref.get() to be valid, since spawning objects can be done recursively, and
    // this can trigger the underlying datastructure to reallocate memory etc. so we need to retrieve it again
    int uid = -1;
    meta_callback(created_shape_copy, [&](const auto& cc) {
      uid = cc.meta_cref().unique_id();
    });
    std::optional<std::reference_wrapper<data_staging::shape_t>> shape_ref_opt;
    for (auto& v : gen_.scenes_.instantiated_objects_current_scene()) {
      meta_callback(v, [&](const auto& ccc) {
        if (ccc.meta_cref().unique_id() == uid) {
          shape_ref_opt = v;
        }
      });
    }
    auto shape_ref = *shape_ref_opt;

    std::vector<std::reference_wrapper<data_staging::shape_t>> new_stack;
    recursively_build_stack_for_object(
        new_stack, shape_ref.get(), gen_.next_instance_map, gen_.scenes_.instantiated_objects_current_scene());
    // reverse new_stack
    std::reverse(new_stack.begin(), new_stack.end());

    gen_.positioner_.handle_rotations(shape_ref.get(), new_stack);

    bool destroyed = gen_.interactor_.destroy_if_duplicate(unique_group, shape_ref.get());

    if (destroyed) {
      return std::nullopt;
    }
  }
  // TODO: simply return here some boolean whether or not this guy is part of a uniqueness group
  // then the caller can do something like If (is_unique) { ... dedupe logic ...  }
  return std::make_tuple(instance, *shape_ref, *shape_copy);
}

void instantiator::_instantiate_object(v8_interact& i,
                                       std::optional<v8::Local<v8::Object>> scene_obj,
                                       v8::Local<v8::Object> object_prototype,
                                       v8::Local<v8::Object> new_instance,
                                       int64_t level,
                                       const std::string& namespace_) {
  v8::Isolate* isolate = i.get_isolate();

  i.recursively_copy_object(new_instance, object_prototype);

  if (!namespace_.empty()) {
    i.set_field(new_instance, "namespace", v8_str(i.get_context(), namespace_));
  }

  if (scene_obj) {
    instantiate_object_copy_fields(i, *scene_obj, new_instance);
  }

  i.set_field(new_instance, "subobj", v8::Array::New(isolate));
  i.set_field(new_instance, "level", v8::Number::New(isolate, level));
  {
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_int_distribution<int> distribution{'a', 'z'};
    static auto generate_len = 6;
    static std::string rand_str(generate_len, '\0');
    for (auto& dis : rand_str) dis = distribution(generator);
    i.set_field(new_instance, "__random_hash__", v8_str(i.get_context(), rand_str));
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

  // Ensure we have some other fields
  for (auto str : {"x", "y", "x2", "y2"}) {
    if (!i.has_field(new_instance, str)) {
      i.set_field(new_instance, str, v8::Number::New(i.get_isolate(), 0));
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

  auto the_fun = i.get_fun("__spawn__");
  i.set_fun(new_instance, "spawn", the_fun);
}

template <typename T>
void instantiator::write_back_copy(T& copy) {
  size_t index = 0;
  for (auto& instance : gen_.scenes_.instantiated_objects_current_scene()) {
    meta_callback(instance, [&]<typename TS>(TS& shape) {
      if (shape.meta_cref().unique_id() == copy.meta_cref().unique_id()) {
        gen_.scenes_.instantiated_objects_current_scene()[index] = copy;
      }
    });
    index++;
  }
}

}  // namespace interpreter
