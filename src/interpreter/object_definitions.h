/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <optional>
#include <unordered_map>

#include "util/v8_wrapper.hpp"

namespace interpreter {

class object_definitions {
public:
  void clear();

  bool contains(const std::string& name);
  std::optional<v8::Local<v8::Object>> get(const std::string& name, bool use_new = false);
  v8::Persistent<v8::Object>& get_persistent(const std::string& name);

  void update(const std::string& name, v8::Local<v8::Object> object_definition);

private:
  std::unordered_map<std::string, v8::Persistent<v8::Object>> object_definitions_map;
};

}  // namespace interpreter
