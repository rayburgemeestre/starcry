/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "positioner.h"
#include <cstdint>

#include "generator.h"

namespace interpreter {

positioner::positioner(generator& gen) : gen_(gen) {}

void positioner::update_object_positions() {
  int64_t scenesettings_from_object_id = -1;
  int64_t scenesettings_from_object_id_level = -1;

  for (auto& abstract_shape : gen_.scenes_.next_shapes_current_scene()) {
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      if constexpr (std::is_same_v<T, data_staging::script>) {
        // TODO: this strategy does not support nested script objects
        // TODO: we need to use stack for that
        scenesettings_from_object_id = shape.meta_cref().unique_id();
        scenesettings_from_object_id_level = shape.meta_cref().level();
      } else if (scenesettings_from_object_id_level == shape.meta_cref().level()) {
        scenesettings_from_object_id = -1;
        scenesettings_from_object_id_level = -1;
      }

      if (scenesettings_from_object_id == -1) {
        gen_.update_time(abstract_shape, shape.meta_cref().id(), gen_.scenes_.scenesettings);
      } else {
        // TODO:
        gen_.update_time(
            abstract_shape, shape.meta_cref().id(), gen_.scenes_.scenesettings_objs[scenesettings_from_object_id]);
      }

      // TODO: this was an interesting hack..
      // should not be needed. I don't think scripts should be able to customize the scale for now.
      // scalesettings.video_scale_next = i.double_number(video, "scale", 1.0);

      auto angle = shape.generic_cref().angle();
      if (std::isnan(angle)) {
        angle = 0.;
      }
      double x = 0;
      double y = 0;
      double x2 = 0;
      double y2 = 0;
      double velocity = 0;
      double vel_x = 0;
      double vel_y = 0;
      double vel_x2 = 0;
      double vel_y2 = 0;
      if constexpr (std::is_same_v<T, data_staging::line>) {
        x = shape.line_start_ref().position_cref().x;
        y = shape.line_start_ref().position_cref().y;
        x2 = shape.line_end_ref().position_cref().x;
        y2 = shape.line_end_ref().position_cref().y;
        velocity = shape.movement_line_start_ref().velocity_speed();
        vel_x = shape.movement_line_start_ref().velocity().x;
        vel_y = shape.movement_line_start_ref().velocity().y;
        vel_x2 = shape.movement_line_end_ref().velocity().x;
        vel_y2 = shape.movement_line_end_ref().velocity().y;
      } else {
        x = shape.location_cref().position_cref().x;
        y = shape.location_cref().position_cref().y;
        velocity = shape.movement_cref().velocity_speed();
        vel_x = shape.movement_cref().velocity().x;
        vel_y = shape.movement_cref().velocity().y;
      }

      velocity /= static_cast<double>(gen_.stepper.max_step);
      x += (vel_x * velocity);
      y += (vel_y * velocity);
      x2 += (vel_x2 * velocity);
      y2 += (vel_y2 * velocity);

      if constexpr (std::is_same_v<T, data_staging::line>) {
        shape.line_start_ref().position_ref().x = x;
        shape.line_start_ref().position_ref().y = y;
        shape.line_end_ref().position_ref().x = x2;
        shape.line_end_ref().position_ref().y = y2;
      } else {
        shape.location_ref().position_ref().x = x;
        shape.location_ref().position_ref().y = y;
      }
    });
  }
}

void positioner::update_rotations() {
  auto& stack = gen_.stack;
  const auto handle_pass1 = [&]<typename T>(data_staging::shape_t& abstract_shape, T& shape, data_staging::meta& meta) {
    if (meta.level() >= 0) {
      if (stack.size() <= meta.level()) {
        stack.emplace_back(abstract_shape);
      } else {
        stack[meta.level()] = std::ref(abstract_shape);
      }
    }
    handle_rotations(abstract_shape, stack);
  };

  // first pass transitive xy become set
  stack.clear();
  for (auto& abstract_shape : gen_.scenes_.next_shapes_current_scene()) {
    std::visit(overloaded{[](std::monostate) {},
                          [&](data_staging::circle& c) {
                            handle_pass1(abstract_shape, c, c.meta_ref());
                          },
                          [&](data_staging::line& l) {
                            handle_pass1(abstract_shape, l, l.meta_ref());
                          },
                          [&](data_staging::text& t) {
                            handle_pass1(abstract_shape, t, t.meta_ref());
                          },
                          [&](data_staging::script& s) {
                            handle_pass1(abstract_shape, s, s.meta_ref());
                          }},
               abstract_shape);
  }
}

void positioner::handle_rotations(data_staging::shape_t& shape,
                                  std::vector<std::reference_wrapper<data_staging::shape_t>>& use_stack) {
  vector2d parent;
  vector2d new_position;
  vector2d new_position2;
  bool pivot_found = false;

  const auto handle = [&]<typename T>(T concrete_shape) {
    // we'll iterate over all the parents of the current shape, and the shape itself, starting at the top pos_parent.
    // in case the current shape is at level 3, we iterate over 0, 1, 2, 3.
    vector2d current;
    vector2d current1;
    vector2d current2;
    double current_rotation = 0;
    for (size_t i = 0; i <= concrete_shape.meta_cref().level(); i++) {
      meta_callback(use_stack[i].get(), [&]<typename TP>(TP& parent_shape) {
        if constexpr (std::is_same_v<TP, data_staging::circle>) {
          current = add_vector(current, parent_shape.location_ref().position_ref());
        } else if constexpr (std::is_same_v<TP, data_staging::text>) {
          current = add_vector(current, parent_shape.location_ref().position_ref());
        } else if constexpr (std::is_same_v<TP, data_staging::line>) {
          current1 = add_vector(current, parent_shape.line_start_ref().position_ref());
          current2 = add_vector(current, parent_shape.line_end_ref().position_ref());
          current = add_vector(
              current, {((current1.x - current2.x) / 2) + current2.x, ((current1.y - current2.y) / 2) + current2.y});
        } else if constexpr (std::is_same_v<TP, data_staging::script>) {
          current = add_vector(current, parent_shape.location_ref().position_ref());
        }

        double angle = parent_shape.generic_ref().angle();
        current_rotation += parent_shape.generic_ref().rotate();

        const auto rotate = [&](auto& current, auto& parent) {
          auto angle1 = current_rotation + angle + get_angle(parent.x, parent.y, current.x, current.y);
          angle1 = std::fmod(angle1, 360.);
          auto rads = angle1 * M_PI / 180.0;
          auto ratio = 1.0;
          auto dist = get_distance(parent.x, parent.y, current.x, current.y);
          auto move = dist * ratio * -1;
          // now current will be adjusted with the rotation
          current.x = parent.x + (cos(rads) * move);
          current.y = parent.y + (sin(rads) * move);
        };

        if constexpr (std::is_same_v<TP, data_staging::line>) {
          rotate(current1, parent);
          rotate(current2, parent);
          new_position = current1;
          new_position2 = current2;
        } else {
          rotate(current, parent);
          new_position = current;
        }

        parent = current;

        if (!pivot_found && parent_shape.meta_cref().is_pivot()) {
          // skip to the last element in the stack, i - 1 due to i++ every iteration
          i = concrete_shape.meta_cref().level() - 1;
          pivot_found = true;
        }
      });
    }
  };

  meta_visit(
      shape,
      [&handle, &new_position](data_staging::circle& c) {
        handle(c);
        c.transitive_location_ref().position_ref().x = new_position.x;
        c.transitive_location_ref().position_ref().y = new_position.y;
      },
      [&handle, &new_position, &new_position2](data_staging::line& l) {
        handle(l);
        bool skip_start = false, skip_end = false;
        for (const auto& cascade_in : l.cascades_in()) {
          if (cascade_in.type() == cascade_type::start) {
            skip_start = true;
          }
          if (cascade_in.type() == cascade_type::end) {
            skip_end = true;
          }
        }
        if (!skip_start) {
          l.transitive_line_start_ref().position_ref().x = new_position.x;
          l.transitive_line_start_ref().position_ref().y = new_position.y;
        }
        if (!skip_end) {
          l.transitive_line_end_ref().position_ref().x = new_position2.x;
          l.transitive_line_end_ref().position_ref().y = new_position2.y;
        }
      },
      [&handle, &new_position](data_staging::text& t) {
        handle(t);
        t.transitive_location_ref().position_ref().x = new_position.x;
        t.transitive_location_ref().position_ref().y = new_position.y;
      },
      [&handle, &new_position](data_staging::script& s) {
        handle(s);
        s.transitive_location_ref().position_ref().x = new_position.x;
        s.transitive_location_ref().position_ref().y = new_position.y;
      });
}

}  // namespace interpreter