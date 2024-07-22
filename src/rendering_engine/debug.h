/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <cstdint>
#include <memory>

class image;
struct render_params;
namespace draw_logic {
class draw_logic;
}

void draw_debug(image& target_bmp,
                std::shared_ptr<draw_logic::draw_logic> dl,
                render_params& params,
                uint32_t offset_x,
                uint32_t offset_y,
                uint32_t width,
                uint32_t height,
                double scale_ratio);