/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>

#include "generator_options.hpp"
#include "util/quadtree.h"
#include "util/unique_group.hpp"
#include "util/vector_logic.hpp"

#include "data/toroidal.hpp"
#include "data_staging/shape.hpp"

class frame_stepper;
class generator_context;
class scene_settings;
struct generator_state;

namespace interpreter {
class toroidal_manager;
class object_lookup;
class object_definitions;
class spawner;
class scenes;
class bridges;

class interactor {
  std::map<std::string, quadtree> qts;
  std::map<std::string, quadtree> qts_gravity;
  std::map<std::string, unique_group> unique_groups;

public:
  explicit interactor(std::shared_ptr<generator_context>& genctx,
                      generator_state& state,
                      scenes& scenes,
                      frame_stepper& stepper,
                      toroidal_manager& tm,
                      object_definitions& definitions,
                      object_lookup& object_lookup,
                      spawner& spawner,
                      bridges& bridges);

  void reset();
  void update_interactions();
  bool destroy_if_duplicate(const std::string& ug, data_staging::shape_t& shape);

private:
  void update_object_toroidal(
      data_staging::toroidal& toroidal_data, double& x, double& y, double& diff_x, double& diff_y);
  void handle_collisions(data_staging::shape_t& instance);
  void handle_collision(data_staging::circle& object_bridge,
                        data_staging::circle& instance2,
                        data_staging::shape_t& shape,
                        data_staging::shape_t& shape2);
  void handle_gravity(data_staging::shape_t& instance);
  template <typename T, typename T2>
  void handle_gravity(T& instance,
                      T2& instance2,
                      vector2d& acceleration,
                      double G,
                      double range,
                      double constrain_dist_min,
                      double constrain_dist_max) const;

  std::shared_ptr<generator_context> genctx;
  generator_state& state_;
  scenes& scenes_;
  frame_stepper& stepper_;
  toroidal_manager& toroidal_manager_;
  object_definitions& definitions_;
  object_lookup& object_lookup_;
  spawner& spawner_;
  bridges& bridges_;
};
}  // namespace interpreter
