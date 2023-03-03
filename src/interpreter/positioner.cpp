/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdint>
#include "positioner.h"

#include "generator.h"

#include "data_staging/shape.hpp"

namespace interpreter {

    positioner::positioner(generator& gen) : gen_(gen) {
    }

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
                    gen_.update_time(abstract_shape, shape.meta_cref().id(), gen_.scenes_.scenesettings_objs[scenesettings_from_object_id]);
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
}  // namespace interpreter