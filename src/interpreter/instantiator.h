/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "data_staging/shape.hpp"
#include "util/random.hpp"
#include "util/v8_interact.hpp"

class generator_context;
class generator_context_wrapper;

namespace interpreter {

void instantiate_object_copy_fields(v8_interact& i,
                                    v8::Local<v8::Object> scene_obj,
                                    v8::Local<v8::Object> new_instance);

class object_definitions;
class initializer;
class object_lookup;
class scenes;
class bridges;
class interactor;
class positioner;

class instantiator {
private:
public:
  explicit instantiator(std::shared_ptr<v8_wrapper> context,
                        generator_context_wrapper& genctx,
                        scenes& scenes,
                        bridges& bridges,
                        object_definitions& definitions,
                        initializer& initializer,
                        object_lookup& object_lookup,
                        positioner& positioner,
                        data_staging::attrs& attrs);

  void init(interactor& interactor_);

  void instantiate_additional_objects_from_new_scene(v8::Persistent<v8::Array>& scene_objects,
                                                     int debug_level = 0,
                                                     const data_staging::shape_t* parent_object = nullptr);

  std::optional<std::tuple<v8::Local<v8::Object>, std::reference_wrapper<data_staging::shape_t>, data_staging::shape_t>>
  instantiate_object_from_scene(
      v8_interact& i,
      v8::Local<v8::Object>& scene_object,          // object description from scene to be instantiated
      const data_staging::shape_t* parent_object);  // it's optional parent

  void reset_seeds();

  void create_bookkeeping_for_script_objects(v8::Local<v8::Object> created_instance,
                                             const data_staging::shape_t& created_shape,
                                             int debug_level = 0);

private:
  void _instantiate_object(v8_interact& i,
                           std::optional<v8::Local<v8::Object>> scene_obj,
                           v8::Local<v8::Object> object_prototype,
                           v8::Local<v8::Object> new_instance,
                           int64_t level,
                           const std::string& namespace_);

  template <typename T>
  void write_back_copy(T& copy);

  std::shared_ptr<v8_wrapper> context;
  generator_context_wrapper& genctx;
  scenes& scenes_;
  bridges& bridges_;
  object_definitions& object_definitions_;
  initializer& initializer_;
  interactor* interactor_ = nullptr;
  object_lookup& object_lookup_;
  positioner& positioner_;
  data_staging::attrs& global_attrs_;
  util::random_generator rand_;
  int64_t counter = 0;
};
}  // namespace interpreter
