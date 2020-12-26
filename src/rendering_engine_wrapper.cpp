/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rendering_engine_wrapper.h"
#include "data/color.hpp"
#include "data/settings.hpp"
#include "rendering_engine.hpp"

struct rendering_engine_wrapper_class_data {
  rendering_engine engine;
};

rendering_engine_wrapper::rendering_engine_wrapper() : data(std::make_shared<rendering_engine_wrapper_class_data>()) {}

void rendering_engine_wrapper::initialize() {
  data->engine.initialize();
}

void rendering_engine_wrapper::render(image &bmp,
                                      const data::color &bg_color,
                                      const std::vector<std::vector<data::shape>> &shapes,
                                      uint32_t offset_x,
                                      uint32_t offset_y,
                                      uint32_t canvas_w,
                                      uint32_t canvas_h,
                                      uint32_t width,
                                      uint32_t height,
                                      double scale,
                                      bool verbose,
                                      data::settings &settings) {
  return data->engine.render(
      bmp, bg_color, shapes, offset_x, offset_y, canvas_w, canvas_h, width, height, scale, verbose, settings);
}

void rendering_engine_wrapper::write_image(image &bmp, const std::string &filename) {
  return data->engine.write_image(bmp, filename);
}
