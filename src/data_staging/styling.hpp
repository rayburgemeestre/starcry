/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "data/blending_type.hpp"

#include "util/logger.h"

namespace data_staging {
class styling {
private:
  std::string gradient_;
  std::string texture_;
  std::vector<std::tuple<double, std::string>> gradients_;
  std::shared_ptr<v8::Persistent<v8::Array>> gradients_obj_ = nullptr;
  std::vector<std::tuple<double, std::string>> textures_;
  data::blending_type blending_type_ = data::blending_type::normal;

public:
  styling() {
    if (gradients_obj_ == nullptr) {
      gradients_obj_ = std::make_shared<v8::Persistent<v8::Array>>();
      (*gradients_obj_).Reset(v8::Isolate::GetCurrent(), v8::Array::New(v8::Isolate::GetCurrent()));
    }
  }
  std::string gradient() const {
    return gradient_;
  }
  data::blending_type blending_type() const {
    return blending_type_;
  }

  void set_gradient(std::string_view gradient) {
    gradient_ = gradient;
  }

  v8::Local<v8::Array> get_gradients_obj() {
    return (*gradients_obj_).Get(v8::Isolate::GetCurrent());
  }

  decltype(gradients_)& get_gradients_ref() {
    return gradients_;
  }

  const decltype(gradients_)& get_gradients_cref() const {
    return gradients_;
  }

  void add_gradient(double value, const std::string& color) {
    v8_interact i;
    v8::Local<v8::Array> keyvalue = v8::Array::New(v8::Isolate::GetCurrent(), 2);
    i.set_field(keyvalue, size_t(0), v8pp::to_v8(i.get_isolate(), value));
    i.set_field(keyvalue, size_t(1), v8pp::to_v8(i.get_isolate(), color));
    i.call_fun(gradients_obj_->Get(i.get_isolate()), "push", keyvalue);
    gradients_.emplace_back(value, color);
  }

  std::string texture() const {
    return texture_;
  }

  void set_texture(std::string_view texture) {
    texture_ = texture;
  }

  const decltype(textures_)& get_textures_cref() const {
    return textures_;
  }

  void commit(v8::Persistent<v8::Object>& instance) {
    gradients_.clear();
    v8_interact i;
    auto gradient_array = (*gradients_obj_).Get(i.get_isolate()).As<v8::Array>();
    for (size_t k = 0; k < gradient_array->Length(); k++) {
      auto gradient_data = i.get_index(gradient_array, k).As<v8::Array>();
      if (!gradient_data->IsArray()) {
        continue;
      }
      auto opacity = i.double_number(gradient_data, size_t(0));
      // TODO: Fix namespace
      auto gradient_id = /* namespace_name +*/ i.str(gradient_data, size_t(1));
      gradients_.emplace_back(opacity, gradient_id);
    }
  }
};
}  // namespace data_staging