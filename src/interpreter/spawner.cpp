/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spawner.h"
#include "abort_exception.hpp"
#include "instantiator.h"
#include "object_definitions.h"
#include "object_lookup.h"
#include "scenes.h"
#include "util/generator_context.h"

// #define DEBUG2

namespace interpreter {
spawner::spawner(generator_context_wrapper& genctx,
                 object_definitions& definitions,
                 instantiator& instantiator,
                 object_lookup& object_lookup,
                 scenes& scenes)
    : genctx(genctx),
      object_definitions_(definitions),
      instantiator_(instantiator),
      object_lookup_(object_lookup),
      scenes_(scenes) {}

int64_t spawner::spawn_object(data_staging::shape_t& spawner, v8::Local<v8::Object> obj) {
  auto& i = genctx.get()->i();

  auto instantiated_object = instantiator_.instantiate_object_from_scene(i, obj, &spawner);
  if (!instantiated_object) {
    return -1;
  }
  auto [created_instance, shape_ref_unusable, created_shape_copy] = *instantiated_object;
  instantiator_.create_bookkeeping_for_script_objects(created_instance, created_shape_copy);

  return i.integer_number(created_instance, "unique_id");
}

int64_t spawner::spawn_object2(data_staging::shape_t& spawner, v8::Local<v8::Object> line_obj, int64_t obj1) {
  auto& i = genctx.get()->i();
  auto uid = i.integer_number(line_obj, "unique_id");

  // create __point__ object definition
  if (!object_definitions_.contains("__point__")) {
    auto self_def = v8::Object::New(i.get_isolate());
    i.set_field(self_def, "x", v8::Number::New(i.get_isolate(), 0));
    i.set_field(self_def, "y", v8::Number::New(i.get_isolate(), 0));
    i.set_field(genctx.get()->objects.Get(i.get_isolate()), "__point__", self_def);
    object_definitions_.update("__point__", self_def);
  }

  // spawn one, we do this so we can get full transitive x and y for the line start
  // without it, the parent for example, will always be one level above the spawner,
  // typically 0,0 for example.
  auto self_obj = v8::Object::New(i.get_isolate());
  i.set_field(self_obj, "id", v8pp::to_v8(i.get_isolate(), "__point__"));
  uid = spawn_object(spawner, self_obj);

  return spawn_object3(spawner, line_obj, obj1, uid);
}

int64_t spawner::spawn_object3(data_staging::shape_t& spawner,
                               v8::Local<v8::Object> line_obj,
                               int64_t obj1,
                               int64_t obj2) {
  auto& i = genctx.get()->i();
  auto instantiated_object = instantiator_.instantiate_object_from_scene(i, line_obj, &spawner);
  if (!instantiated_object) {
    return -1;
  }
  auto [created_instance, shape_ref, created_shape_copy] = *instantiated_object;
  // BEGIN: Temporary code (to try out something
  data_staging::shape_t* obj1o = nullptr;
  data_staging::shape_t* obj2o = nullptr;
  auto find1 = object_lookup_.find(obj1);
  if (find1 != object_lookup_.end()) {
    obj1o = &find1->second.get();
  } else {
    // todo; we need a mapping for this...
    for (auto& newo : scenes_.instantiated_objects_current_scene()) {
      if (auto c = std::get_if<data_staging::circle>(&newo)) {
        if (c->meta_cref().unique_id() == obj1) {
          obj1o = &newo;
        }
      }
    }
  }
  auto find2 = object_lookup_.find(obj2);
  if (find2 != object_lookup_.end()) {
    obj2o = &find2->second.get();
  } else {
    // todo; we need a mapping for this...
    for (auto& newo : scenes_.instantiated_objects_current_scene()) {
      if (auto c = std::get_if<data_staging::circle>(&newo)) {
        if (c->meta_cref().unique_id() == obj2) {
          obj2o = &newo;
        }
      }
    }
  }
  // END
  const auto handle = [](auto& line_o, auto& side_a, auto& side_b) {
    try {
      auto& line_obj = std::get<data_staging::line>(line_o);
      auto& circle_obj1 = std::get<data_staging::circle>(side_a);
      auto& circle_obj2 = std::get<data_staging::circle>(side_b);
      circle_obj1.add_cascade_out(cascade_type::start, line_obj.meta_cref().unique_id());
      circle_obj2.add_cascade_out(cascade_type::end, line_obj.meta_cref().unique_id());
      line_obj.add_cascade_in(cascade_type::start, circle_obj1.meta_cref().unique_id());
      line_obj.add_cascade_in(cascade_type::end, circle_obj2.meta_cref().unique_id());
    } catch (std::bad_variant_access const& ex) {
      logger(WARNING) << "bad variant access connecting objects" << std::endl;
      return;
    }
  };
  handle(shape_ref.get(), *obj1o, *obj2o);
  instantiator_.create_bookkeeping_for_script_objects(created_instance, created_shape_copy);
  return i.integer_number(created_instance, "unique_id");
}

int64_t spawner::spawn_object_at_parent(data_staging::shape_t& spawner, v8::Local<v8::Object> obj) {
  auto& i = genctx.get()->i();
  std::optional<std::reference_wrapper<data_staging::shape_t>> parent;
  meta_callback(spawner, [&]<typename T>(const T& cc) {
    if (cc.meta_cref().level() > 0) {
      parent = object_lookup_.at(cc.meta_cref().parent_uid());
    }
  });
  auto instantiated_object = ([&]() {
    if (!parent) {
      return instantiator_.instantiate_object_from_scene(i, obj, nullptr);
    }
    return instantiator_.instantiate_object_from_scene(i, obj, &((*parent).get()));
  })();
  if (!instantiated_object) {
    return -1;
  }
  auto [created_instance, shape_ref, created_shape_copy] = *instantiated_object;
  instantiator_.create_bookkeeping_for_script_objects(created_instance, created_shape_copy);
  return i.integer_number(created_instance, "unique_id");
}

int64_t spawner::destroy(data_staging::shape_t& caller) {
  int64_t ret = -1;
  meta_callback(caller, [&]<typename T>(T& shape) {
    ret = shape.meta_cref().unique_id();
    shape.meta_ref().set_destroyed();
  });
  return ret;
}

}  // namespace interpreter
