/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "generator/object_bridge.h"

#include "util/v8_interact.hpp"

#include "native_generator.h"

template <typename T>
void object_bridge<T>::push_object(T& c) {
  properties_accessed_ = false;
  shape_stack.push_back(&c);
}

template <typename T>
void object_bridge<T>::pop_object() {
  if (properties_accessed_) {
    shape_stack.back()->properties_ref().commit();
  }
  properties_accessed_ = false;
  shape_stack.pop_back();
}

template <typename shape_class>
void object_bridge<shape_class>::set_generator(native_generator* ptr) {
  generator_ = ptr;
}

template <typename T>
void object_bridge<T>::spawn(v8::Local<v8::Object> obj) {
  data_staging::shape_t var = *shape_stack.back();
  generator_->spawn_object(var, obj);
}

template <typename T>
int64_t object_bridge<T>::get_level() const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->meta().level();
}

template <typename T>
v8::Local<v8::Object> object_bridge<T>::get_properties_ref() const {
  properties_accessed_ = true;
  return shape_stack.back()->properties_ref().properties_ref();
}

template class object_bridge<data_staging::circle>;
template class object_bridge<data_staging::line>;