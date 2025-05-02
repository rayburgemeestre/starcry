/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <v8/v8.h>
#include <map>
#include <string>
#include <unordered_map>

namespace interpreter {

class project_specifications {
private:
  std::map<std::string, std::string> specs_;

public:
  std::string get_spec(const std::string& spec);
  void set_spec(const std::string& spec, const std::string& spec_json_string);
};

}  // namespace interpreter

class v8_interact;
struct specification_field;
using specification_fields = std::unordered_map<std::string, specification_field>;

void validate_field_types(v8_interact& i,
                          v8::Local<v8::Object>& object_to_check,
                          const std::string& object_name,
                          specification_fields& spec);
void validate_unknown_fields(v8_interact& i,
                             v8::Local<v8::Object>& object_to_check,
                             const std::string& object_name,
                             specification_fields& spec);
