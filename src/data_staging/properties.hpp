/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include "util/vector_logic.hpp"
#include "util/logger.h"
#include "util/v8_interact.hpp"

#include "v8.h"

namespace data_staging {
class properties {
private:
  v8::Local<v8::Object> properties_;
  // std::map<std::string, std::variant<int, double, std::string>> real_properties_;
  double z_ = 0;

public:

  properties() {
    properties_ = v8::Object::New(v8::Isolate::GetCurrent());
  }

  v8::Local<v8::Object> properties_ref() {
    return properties_;
  }

  void commit() {
    v8_interact i(v8::Isolate::GetCurrent());
    auto obj_fields = i.prop_names(properties_);
    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_name_str = i.str(obj_fields, k);
      auto field_value = i.get(properties_, field_name);
      // i.set_field(props, field_name, field_value);
      logger(INFO) << "commit somewhere? maybe not.. " << field_name_str << std::endl;
    }
  }

  void for_each(std::function<void(const std::string, v8::Local<v8::Value> value)> callback) {
    v8_interact i(v8::Isolate::GetCurrent());
    auto obj_fields = i.prop_names(properties_);
    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_name_str = i.str(obj_fields, k);
      auto field_value = i.get(properties_, field_name);
      callback(field_name_str, field_value);
    }
  }

};
}  // namespace data_staging