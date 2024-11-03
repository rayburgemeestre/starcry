/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "data_staging/shape.hpp"

namespace interpreter {

class scenes;

class debug_printer {
public:
  explicit debug_printer(scenes& scenes);
  void debug_print_all();
  void debug_print_next();
  void debug_print(std::vector<data_staging::shape_t>& shapes);

private:
  scenes& scenes_;
};
}  // namespace interpreter
