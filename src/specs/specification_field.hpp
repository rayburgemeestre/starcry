/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <v8.h>
#include <string>

struct specification_field {
  std::string type;
  v8::Local<v8::Value> default_value;
  std::string description;
};

using specification_fields = std::unordered_map<std::string, specification_field>;
