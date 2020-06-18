/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rendering_engine_wrapper.h"
#include "primitives.h"
#include "rendering_engine.hpp"

struct rendering_engine_wrapper_class_data {
  rendering_engine engine;
};

rendering_engine_wrapper::rendering_engine_wrapper() : data(std::make_shared<rendering_engine_wrapper_class_data>()) {}

void rendering_engine_wrapper::initialize() {
  data->engine.initialize();
}

void rendering_engine_wrapper::render(image bmp,
                                      data::color &bg_color,
                                      shapes_t &shapes,
                                      uint32_t offset_x,
                                      uint32_t offset_y,
                                      uint32_t canvas_w,
                                      uint32_t canvas_h,
                                      uint32_t width,
                                      uint32_t height,
                                      double scale) {
  return data->engine.render(bmp, bg_color, shapes, offset_x, offset_y, canvas_w, canvas_h, width, height, scale);
}

void rendering_engine_wrapper::write_image(image bmp, const std::string &prefix) {
  return data->engine.write_image(bmp, prefix);
}

std::vector<uint32_t> rendering_engine_wrapper::serialize_bitmap2(image bitmap, uint32_t width, uint32_t height) {
  return data->engine.serialize_bitmap2(bitmap, width, height);
}

using image = ALLEGRO_BITMAP *;
image rendering_engine_wrapper::unserialize_bitmap2(std::vector<uint32_t> &pixels, uint32_t width, uint32_t height) {
  return data->engine.unserialize_bitmap2(pixels, width, height);
}
