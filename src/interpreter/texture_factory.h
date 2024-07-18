/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <memory>
#include <string>
#include "util/v8_interact.hpp"

class v8_interact;

namespace data {
struct texture;
}

namespace interpreter {

class texture_factory {
public:
  static data::texture create_from_object(v8_interact& i, v8::Local<v8::Object>& texture_settings);
};

}  // namespace interpreter
