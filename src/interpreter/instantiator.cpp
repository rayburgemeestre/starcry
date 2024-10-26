/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "instantiator.h"
#include "abort_exception.hpp"
#include "generator.h"

// #define DEBUG2

namespace interpreter {

void instantiate_object_copy_fields(v8_interact& i,
                                    v8::Local<v8::Object> scene_obj,
                                    v8::Local<v8::Object> new_instance) {
  // @add_field@
  for (auto field : {"angle",
                     "blending_type",
                     "collision_group",
                     "duration",
                     "gradient",
                     "gradients",
                     "gravity_group",
                     "hue",
                     "id",
                     "longest_diameter",
                     "mass",
                     "opacity",
                     "pivot",
                     "radius",
                     "radiussize",
                     "rotate",
                     "scale",
                     "recursive_scale",
                     "seed",
                     "shortest_diameter",
                     "text",
                     "text_align",
                     "text_fixed",
                     "text_font",
                     "text_size",
                     "texture",
                     "texture_3d",
                     "texture_effect",
                     "texture_offset_x",
                     "texture_offset_y",
                     "zernike_type",
                     "texture_effect",
                     "unique_group",
                     "vel_x",
                     "vel_x2",
                     "vel_y",
                     "vel_y2",
                     "velocity",
                     "x",
                     "x2",
                     "y",
                     "y2"}) {
    i.copy_field_if_exists(new_instance, field, scene_obj);
  }
}

void recursively_build_stack_for_object(auto& new_stack,
                                        auto& shape,
                                        auto& object_lookup,
                                        std::vector<data_staging::shape_t>& instantiated_objs,
                                        int level = 0) {
  meta_callback(shape, [&](const auto& cc) {
    new_stack.emplace_back(shape);
    if (cc.meta_cref().level() > 0) {
      if (object_lookup.find(cc.meta_cref().parent_uid()) != object_lookup.end()) {
        recursively_build_stack_for_object(new_stack,
                                           object_lookup.at(cc.meta_cref().parent_uid()).get(),
                                           object_lookup,
                                           instantiated_objs,
                                           level + 1);
      } else {
        for (auto& v : instantiated_objs) {
          meta_callback(v, [&](const auto& ccc) {
            if (ccc.meta_cref().unique_id() == cc.meta_cref().parent_uid()) {
              recursively_build_stack_for_object(new_stack, v, object_lookup, instantiated_objs, level + 1);
            }
          });
        }
      }
    }
  });
}

instantiator::instantiator(generator& gen, object_definitions& definitions) : gen_(gen), definitions_(definitions) {}

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

    auto instantiated_object = instantiate_object_from_scene(i, scene_obj, parent_object);
    if (instantiated_object) {
      auto [created_instance, shape_ref, shape_copy] = *instantiated_object;
      gen_.create_bookkeeping_for_script_objects(created_instance, shape_copy, debug_level + 1);
    }
  }
}

std::optional<std::tuple<v8::Local<v8::Object>, std::reference_wrapper<data_staging::shape_t>, data_staging::shape_t>>
instantiator::instantiate_object_from_scene(
    v8_interact& i,
    v8::Local<v8::Object>& scene_object,           // object description from scene to be instantiated
    const data_staging::shape_t* parent_object) {  // it's optional parent
  v8::Isolate* isolate = i.get_isolate();

  int64_t current_level = 0;
  auto parent_object_ns = std::string("");
  int64_t parent_uid = -1;
  double parent_recursive_scale = 1.;

  if (parent_object) {
    meta_callback(*parent_object, [&]<typename T>(const T& cc) {
      current_level = cc.meta_cref().level() + 1;
      parent_object_ns = cc.meta_cref().namespace_name();
      parent_uid = cc.meta_cref().unique_id();
      parent_recursive_scale = cc.generic_cref().recursive_scale();
    });
  }

  // lookup the object prototype to be instantiated
  auto object_id = i.str(scene_object, "id", "");
  if (object_id != "__point__") {
    object_id = parent_object_ns + object_id;
  }
  auto object_prototype = v8_index_object(i.get_context(), gen_.genctx->objects, object_id).template As<v8::Object>();

  if (object_prototype->IsNullOrUndefined() || !object_prototype->IsObject()) {
    logger(WARNING) << "cannot instantiate object id: " << object_id << ", does not exist" << std::endl;
    throw std::runtime_error(fmt::format("cannot instantiate object id: {}, does not exist", object_id));
  }

  // create a new javascript object
  v8::Local<v8::Object> instance = v8::Object::New(isolate);

  // TODO: make sure this is the only source..., and get rid of genctx->objects usage
  if (!object_prototype->IsObject()) {
    object_prototype = *definitions_.get(object_id);
  }

  // instantiate the prototype into newly allocated javascript object
  _instantiate_object(i, scene_object, object_prototype, instance, current_level, parent_object_ns);

  // give it a unique id (it already has been assigned a random_hash for debugging purposes
  i.set_field(instance, "unique_id", v8::Number::New(i.get_isolate(), ++counter));
  i.set_field(instance, "parent_uid", v8::Number::New(i.get_isolate(), parent_uid));

  // TODO: something is not right here.. Why can't we just set_field with existing recursive scale combined..
  if (!i.has_field(instance, "recursive_scale") && parent_recursive_scale != 1.) {
    double existing_recursive_scale = 1.;  // i.double_number(instance, "recursive_scale", 1.);
    i.set_field(instance,
                "recursive_scale",
                v8::Number::New(i.get_isolate(), existing_recursive_scale * parent_recursive_scale));
  }

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

  auto& obj = definitions_.get_persistent(object_id);
  const auto type = i.str(obj, "type", "");
  bool check_uniqueness = false;
  std::string unique_group = "";

  // @add_field@
  const auto initialize = [&]<typename T>(T& c, auto& bridge) {
    c.meta_ref().set_level(current_level);
    c.meta_ref().set_parent_uid(parent_uid);
    c.meta_ref().set_pivot(i.boolean(instance, "pivot"));
    c.meta_ref().set_random_hash(i.str(instance, "random_hash"));
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
    c.generic_ref().set_recursive_scale(i.double_number(instance, "recursive_scale", 1));
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      if (i.has_field(instance, "texture")) {
        c.styling_ref().set_texture(i.str(instance, "texture"));
      }
      if (i.has_field(instance, "texture_3d")) {
        c.styling_ref().set_texture_3d(i.integer_number(instance, "texture_3d"));
      }
      if (i.has_field(instance, "texture_offset_x")) {
        c.styling_ref().set_texture_offset_x(i.integer_number(instance, "texture_offset_x"));
      }
      if (i.has_field(instance, "texture_offset_y")) {
        c.styling_ref().set_texture_offset_y(i.integer_number(instance, "texture_offset_y"));
      }
      if (i.has_field(instance, "effect")) {
        c.styling_ref().set_texture_effect(i.integer_number(instance, "effect"));
      }
      if (i.has_field(instance, "zernike_type")) {
        c.styling_ref().set_zernike_type(i.integer_number(instance, "zernike_type"));
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

    if (i.has_field(instance, "attrs")) {
      auto attrs = i.v8_obj(instance, "attrs");
      auto obj_fields = i.prop_names(attrs);
      for (size_t k = 0; k < obj_fields->Length(); k++) {
        auto field_name = i.get_index(obj_fields, k);
        auto field_name_str = i.str(obj_fields, k);
        auto field_value = i.get(attrs, field_name);
        i.set_field(c.properties_ref().properties_ref(), field_name, field_value);
        if (field_value->IsString()) {
          c.attrs_ref().set(field_name_str, v8_str(i.get_isolate(), field_value.As<v8::String>()));
          if constexpr (std::is_same_v<T, data_staging::script>) {
            gen_.global_attrs_.set(field_name_str, v8_str(i.get_isolate(), field_value.As<v8::String>()));
          }
        } else if (field_value->IsNumber()) {
          c.attrs_ref().set(field_name_str, field_value.As<v8::Number>()->Value());
          if constexpr (std::is_same_v<T, data_staging::script>) {
            gen_.global_attrs_.set(field_name_str, field_value.As<v8::Number>()->Value());
          }
        }
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

#ifdef DEBUG2
      if (!gen_.object_definitions_map.contains(object_id)) {
        throw abort_exception(fmt::format("object_id ({}) not found in definitions map", object_id));
      }
#endif
      auto obj = *definitions_.get(object_id);
      i.call_fun(obj,                 // object definition
                 bridge->instance(),  // bridged object is "this"
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

  } else if (type == "ellipse") {
    data_staging::ellipse e(object_id,
                            counter,
                            vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                            i.double_number(instance, "longest_diameter", i.double_number(instance, "a")),
                            i.double_number(instance, "shortest_diameter", i.double_number(instance, "b")),
                            i.double_number(instance, "radiussize"));

    if (type.empty()) e.generic_ref().set_opacity(0);

    e.movement_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                  i.double_number(instance, "vel_y", 0),
                                  i.double_number(instance, "velocity", 0));

    e.meta_ref().set_namespace(parent_object_ns);

    e.styling_ref().set_seed(i.integer_number(instance, "seed", 0));
    e.styling_ref().set_blending_type(i.integer_number(instance, "blending_type"));

    initialize(e, gen_.bridges_.ellipse());

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
    s.set_radius(i.double_number(instance, "radius", 0));
    s.set_radius_size(i.double_number(instance, "radiussize", 0));
    s.movement_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                  i.double_number(instance, "vel_y", 0),
                                  i.double_number(instance, "velocity", 0));
    s.generic_ref().set_opacity(i.double_number(instance, "opacity", 1));

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
        new_stack, shape_ref.get(), gen_.object_lookup_, gen_.scenes_.instantiated_objects_current_scene());
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

void instantiator::reset_seeds() {
  rand_.set_seed(0);
  counter = 0;
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
    static auto generate_len = 6;
    static std::string rand_str(generate_len, '\0');
    for (auto& dis : rand_str) dis = 'a' + char(rand_.get() * ('z' - 'a'));
    i.set_field(new_instance, "random_hash", v8_str(i.get_context(), rand_str));
  }
  i.set_field(new_instance, "__instance__", v8::Boolean::New(isolate, true));

  // Ensure we have a props object in the new obj
  if (!i.has_field(new_instance, "props")) {
    i.set_field(new_instance, "props", v8::Object::New(i.get_isolate()));
  }

  // Ensure we have an attrs object in the new obj
  if (!i.has_field(new_instance, "attrs")) {
    i.set_field(new_instance, "attrs", v8::Object::New(i.get_isolate()));
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

  // Do the same for attributes
  if (scene_obj) {
    auto props = i.v8_obj(new_instance, "attrs");
    auto scene_props = i.v8_obj(*scene_obj, "attrs");
    auto obj_fields = i.prop_names(scene_props);

    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_value = i.get(scene_props, field_name);
      i.set_field(props, field_name, field_value);
    }
  }
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
