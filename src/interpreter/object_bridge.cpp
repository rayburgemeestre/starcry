/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "interpreter/object_bridge.h"
#include "data_staging/shape.hpp"
#include "generator.h"
#include "util/v8_interact.hpp"

template <typename T>
void object_bridge<T>::push_object(T& c) {
  properties_accessed_ = false;
  gradients_accessed_ = false;
  shape_stack.push_back(&c);
}

template <typename T>
void object_bridge<T>::pop_object() {
  if (properties_accessed_) {
    shape_stack.back()->properties_ref().for_each([](const std::string, v8::Local<v8::Value> value) {
      // do something with the properties
    });
    // commit gradients data
  }
  if constexpr (!std::is_same_v<T, data_staging::script>) {
    if (gradients_accessed_) {
      // shape_stack.back()->styling_ref().set_gradients_dirty();
      auto& defs = this->generator_->get_object_definitions_ref();
      auto obj_id = shape_stack.back()->meta_cref().id();
      auto find = defs.find(obj_id);
      if (find != defs.end()) {
        auto& def = find->second;
        shape_stack.back()->styling_ref().commit(def);
      }
    }
  }
  properties_accessed_ = false;
  gradients_accessed_ = false;
  shape_stack.pop_back();
}

template <typename T>
v8::Local<v8::Value> object_bridge<T>::get_attr(v8::Local<v8::String> obj) {
  auto& cc = *shape_stack.back();
  v8::Local<v8::Value> f = cc.attrs_ref().get(v8_str(v8::Isolate::GetCurrent(), obj));
  if (f->IsString()) {
    return f;
  }
  return v8::Undefined(v8::Isolate::GetCurrent());
}

template <typename T>
void object_bridge<T>::set_attr(v8::Local<v8::String> key, v8::Local<v8::String> obj) {
  auto& cc = *shape_stack.back();
  cc.attrs_ref().set(v8_str(v8::Isolate::GetCurrent(), key), v8_str(v8::Isolate::GetCurrent(), obj));
}

template <typename T>
int64_t object_bridge<T>::spawn(v8::Local<v8::Object> obj) {
  data_staging::shape_t var = *shape_stack.back();
  return generator_->spawn_object(var, obj);
}

template <typename T>
int64_t object_bridge<T>::spawn2(v8::Local<v8::Object> line_obj, int64_t obj1) {
  data_staging::shape_t var = *shape_stack.back();
  return generator_->spawn_object2(var, line_obj, obj1);
}

template <typename T>
int64_t object_bridge<T>::spawn3(v8::Local<v8::Object> line_obj, int64_t obj1, int64_t obj2) {
  data_staging::shape_t var = *shape_stack.back();
  return generator_->spawn_object3(var, line_obj, obj1, obj2);
}

template <typename T>
int64_t object_bridge<T>::spawn_parent(v8::Local<v8::Object> line_obj) {
  data_staging::shape_t var = *shape_stack.back();
  return generator_->spawn_object_at_parent(var, line_obj);
}

template <typename T>
int64_t object_bridge<T>::destroy() {
  data_staging::shape_t var = *shape_stack.back();
  // this operates on a copy
  auto ret = generator_->destroy(var);
  // we swap the copy with the real one, to keep changes (such as set_destroyed())
  std::swap(std::get<T>(var), *shape_stack.back());
  return ret;
}

template <typename T>
int64_t object_bridge<T>::get_level() const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->meta_cref().level();
}

template <typename T>
v8::Persistent<v8::Object>& object_bridge<T>::get_properties_ref() const {
  properties_accessed_ = true;
  return shape_stack.back()->properties_ref().properties_ref();
}

template <typename T>
v8::Local<v8::Object> object_bridge<T>::get_properties_local_ref() const {
  properties_accessed_ = true;
  return shape_stack.back()->properties_ref().properties_ref().Get(v8::Isolate::GetCurrent());
}

template <typename T>
v8::Local<v8::Array> object_bridge<T>::get_gradients_local_ref() const {
  if constexpr (std::is_same_v<T, data_staging::script>) {
    return v8::Array::New(v8::Isolate::GetCurrent(), 0);
  } else {
    gradients_accessed_ = true;
    return shape_stack.back()->styling_ref().get_gradients_obj();
  }
}

template <typename T>
std::vector<std::tuple<double, std::string>>& object_bridge<T>::get_gradients_ref() const {
  if constexpr (std::is_same_v<T, data_staging::script>) {
    static std::vector<std::tuple<double, std::string>> empty;
    return empty;
  } else {
    gradients_accessed_ = true;
    return shape_stack.back()->styling_ref().get_gradients_ref();
  }
}

template <typename T>
void object_bridge<T>::set_texture(const std::string& value) const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->styling_ref().set_texture(value);
}

template <typename T>
void object_bridge<T>::set_texture_3d(data::texture_3d value) const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->styling_ref().set_texture_3d(value);
}

template <typename T>
std::string object_bridge<T>::get_texture() const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->styling_cref().texture();
}

template <typename T>
data::texture_3d object_bridge<T>::get_texture_3d() const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->styling_cref().texture_3d();
}

// @add_field@
template <typename T>
double object_bridge<T>::get_texture_offset_x() const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->styling_cref().texture_offset_x();
}

template <typename T>
double object_bridge<T>::get_texture_offset_y() const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->styling_cref().texture_offset_y();
}

template <typename T>
void object_bridge<T>::set_texture_offset_x(double value) const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->styling_ref().set_texture_offset_x(value);
}

template <typename T>
void object_bridge<T>::set_texture_offset_y(double value) const {
  data_staging::shape_t var = *shape_stack.back();
  return shape_stack.back()->styling_ref().set_texture_offset_y(value);
}

template <typename T>
v8::Persistent<v8::Object>& object_bridge<T>::instance() {
  return *instance_;
}

template class object_bridge<data_staging::circle>;
template class object_bridge<data_staging::ellipse>;
template class object_bridge<data_staging::line>;
template class object_bridge<data_staging::script>;
template class object_bridge<data_staging::text>;
