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

public:
  void reinitialize() {
    // there are some problems in making a copy constructor that does this
    // it appears in some cases we have objects copied, at times where
    // v8 would give errors, better to have an explicit reinitialize()
    // function such as this one.

    // see if we can do without this
    decltype(properties_) old_properties = properties_;

    // replace
    properties_ = nullptr;
    init_properties();

    // copy everything from old
    v8_interact i;
    i.recursively_copy_object(*properties_, *old_properties);
  }

  properties() {
    init_properties();
  }

  v8::Persistent<v8::Object>& properties_ref() {
    return *properties_;
  }

  void for_each(std::function<void(const std::string, v8::Local<v8::Value> value)> callback) {
    v8_interact i;
    auto obj_fields = i.prop_names(*properties_);
    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_name_str = i.str(obj_fields, k);
      auto field_value = i.get(*properties_, field_name);
      callback(field_name_str, field_value);
    }
  }

private:
  void init_properties() {
    if (properties_ == nullptr) {
      if (v8::Isolate::GetCurrent() == nullptr) {
        // Isolate is null, v8 not loaded, assuming unit tests.
        return;
      }
      properties_ = std::make_shared<v8::Persistent<v8::Object>>();
      (*properties_).Reset(v8::Isolate::GetCurrent(), v8::Object::New(v8::Isolate::GetCurrent()));
    }
  }
};
}  // namespace data_staging