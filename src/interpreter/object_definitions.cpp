/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "object_definitions.h"

namespace interpreter {

void object_definitions::clear() {
  object_definitions_map.clear();
}

bool object_definitions::contains(const std::string& name) {
  return object_definitions_map.contains(name);
}

std::optional<v8::Local<v8::Object>> object_definitions::get(const std::string& name, bool use_new) {
  auto it = object_definitions_map.find(name);
  if (it != object_definitions_map.end()) {
    if (use_new) {
      auto new_object = v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), it->second);
      return new_object;
    }
    return it->second.Get(v8::Isolate::GetCurrent());
  }
  return std::nullopt;
}

v8::Persistent<v8::Object>& object_definitions::get_persistent(const std::string& name) {
  auto it = object_definitions_map.find(name);
  if (it != object_definitions_map.end()) {
    return it->second;
  }
  throw std::runtime_error("Object definition not found");
}

void object_definitions::update(const std::string& name, v8::Local<v8::Object> object_definition) {
  object_definitions_map[name].Reset(v8::Isolate::GetCurrent(), object_definition);
}

}  // namespace interpreter