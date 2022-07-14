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
  shape_stack.push_back(&c);
}

template <typename T>
void object_bridge<T>::pop_object() {
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

template class object_bridge<data_staging::circle>;
template class object_bridge<data_staging::line>;