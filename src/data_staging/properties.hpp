/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <memory>

#include "util/logger.h"
#include "util/v8_interact.hpp"
#include "util/vector_logic.hpp"

#include "v8.h"

namespace data_staging {
class properties {
private:
  std::shared_ptr<v8::Persistent<v8::Object>> properties_ = nullptr;
  double z_ = 0;

public:
  properties() {
    if (properties_ == nullptr) {
      properties_ = std::make_shared<v8::Persistent<v8::Object>>();
      (*properties_).Reset(v8::Isolate::GetCurrent(), v8::Object::New(v8::Isolate::GetCurrent()));
    }
  }

  v8::Persistent<v8::Object>& properties_ref() {
    return *properties_;
  }

  void for_each(std::function<void(const std::string, v8::Local<v8::Value> value)> callback) {
    v8_interact i(v8::Isolate::GetCurrent());
    auto obj_fields = i.prop_names(*properties_);
    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_name_str = i.str(obj_fields, k);
      auto field_value = i.get(*properties_, field_name);
      callback(field_name_str, field_value);
    }
  }
};
}  // namespace data_staging