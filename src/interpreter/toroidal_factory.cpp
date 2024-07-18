/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "toroidal_factory.h"

#include "data/toroidal.hpp"

namespace interpreter {

data::toroidal toroidal_factory::create_from_object(v8_interact& i, v8::Local<v8::Object>& toroidal_settings) {
  // auto type = i.str(toroidal_settings, "type");
  data::toroidal new_toroidal;
  new_toroidal.width = i.integer_number(toroidal_settings, "width");
  new_toroidal.height = i.integer_number(toroidal_settings, "height");
  new_toroidal.x = i.integer_number(toroidal_settings, "x");
  new_toroidal.y = i.integer_number(toroidal_settings, "y");
  return new_toroidal;
}

}  // namespace interpreter
