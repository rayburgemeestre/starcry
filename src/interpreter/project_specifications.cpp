/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "project_specifications.h"

namespace interpreter {

std::string project_specifications::get_spec(const std::string& spec) {
  return specs_[spec];
}

void project_specifications::set_spec(const std::string& spec, const std::string& spec_json_string) {
  specs_[spec] = spec_json_string;
}

}  // namespace interpreter

#include "specs/specification_field.hpp"
#include "util/logger.h"
#include "util/v8_interact.hpp"

void validate_field_types(v8_interact& i,
                          v8::Local<v8::Object>& object_to_check,
                          const std::string& object_name,
                          specification_fields& spec) {
  auto context = i.get_context();

  for (const auto& [key, field] : spec) {
    if (!i.has_field(object_to_check, key)) {
      logger(DEBUG) << "Field not set: '" << key << "' in " << object_name << std::endl;
      continue;
    }

    auto value = i.get(object_to_check, key);

    bool type_valid = false;
    if (field.type == "int" || field.type == "float") {
      type_valid = value->IsNumber();
    } else if (field.type == "bool") {
      type_valid = value->IsBoolean();
    } else if (field.type == "object") {
      type_valid = value->IsObject();
    }

    if (!type_valid) {
      logger(WARNING) << "Invalid type for field '" << key << "'. Expected " << field.type
                      << ". Using default value. Fixing in " << object_name << std::endl;
      object_to_check->Set(context, v8_str(i.get_isolate(), key), field.default_value).Check();
    }
  }
}

void validate_unknown_fields(v8_interact& i,
                             v8::Local<v8::Object>& object_to_check,
                             const std::string& object_name,
                             specification_fields& spec) {
  // auto spec = create_video_spec(i);
  auto field_names = object_to_check->GetOwnPropertyNames(i.get_context()).ToLocalChecked();

  for (size_t idx = 0; idx < field_names->Length(); idx++) {
    auto field_name = i.get_index(field_names, idx);
    std::string field_str = v8_str(i.get_isolate(), field_name.As<v8::String>());
    if (spec.find(field_str) == spec.end()) {
      logger(WARNING) << "Unknown " << object_name << " configuration field '" << field_str << "'" << std::endl;
    }
  }
}
