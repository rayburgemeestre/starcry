/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>

#include "util/v8_interact.hpp"

namespace data {
class gradient;
}

namespace interpreter {

class gradient_factory {
public:
  static data::gradient create_from_array(const v8::Local<v8::Array>& positions, v8_interact& i);
  static data::gradient create_from_string(const std::string& color_string);
};

}
