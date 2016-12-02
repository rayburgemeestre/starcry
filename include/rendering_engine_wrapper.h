/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include <memory>
#include <cstdint>

struct rendering_engine_wrapper_class_data;

struct ALLEGRO_BITMAP;

namespace data {
    struct color;
    struct shape;
}

class rendering_engine_wrapper
{
public:
    rendering_engine_wrapper();

    void initialize();

    using image = ALLEGRO_BITMAP *;
    using shapes_t = std::vector<data::shape>;
    void render(image bmp, data::color &bg_color, shapes_t & shapes, uint32_t offset_x, uint32_t offset_y, uint32_t canvas_w, uint32_t canvas_h, double scale);

    std::vector<uint32_t> serialize_bitmap2(image bitmap, uint32_t width, uint32_t height);
    image unserialize_bitmap2(std::vector<uint32_t> &pixels, uint32_t width, uint32_t height);

    std::shared_ptr<rendering_engine_wrapper_class_data> data;
};
