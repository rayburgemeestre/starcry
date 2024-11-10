/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "bridges.h"
#include "data_staging/shape.hpp"
#include "object_definitions.h"

class frame_stepper;
class generator_context;
class scene_settings;

namespace interpreter {
class scenes;
class object_lookup;
class object_definitions;
class bridges;

class positioner {
private:
  std::shared_ptr<generator_context>& genctx;
  scenes& scenes_;
  frame_stepper& stepper_;
  object_definitions& definitions_;
  object_lookup& object_lookup_;
  bridges& bridges_;
  std::vector<std::reference_wrapper<data_staging::shape_t>> stack;

public:
  explicit positioner(std::shared_ptr<generator_context>& genctx,
                      scenes& scenes,
                      frame_stepper& stepper,
                      object_definitions& definitions,
                      object_lookup& object_lookup,
                      bridges& bridges);

  void update_object_positions();
  void update_time(data_staging::shape_t& object_bridge, const std::string& instance_id, scene_settings& scenesettings);
  void update_rotations();

  void handle_rotations(data_staging::shape_t& shape,
                        std::vector<std::reference_wrapper<data_staging::shape_t>>& use_stack);

  void revert_position_updates();
};

}  // namespace interpreter
