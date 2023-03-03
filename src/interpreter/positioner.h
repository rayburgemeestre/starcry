/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "data_staging/shape.hpp"

namespace interpreter {
class generator;

class positioner {
private:
  generator& gen_;

public:
  explicit positioner(generator& gen);
  void update_object_positions();
  void update_rotations();

  void handle_rotations(data_staging::shape_t& shape,
                        std::vector<std::reference_wrapper<data_staging::shape_t>>& use_stack);
};

}  // namespace interpreter
