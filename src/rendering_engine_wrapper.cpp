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

image &rendering_engine_wrapper::render(size_t thread_num,
                                        size_t job_num,
                                        size_t chunk_num,
                                        std::shared_ptr<metrics> &metrics,
                                        const data::color &bg_color,
                                        const std::vector<std::vector<data::shape>> &shapes,
                                        double view_x,
                                        double view_y,
                                        uint32_t offset_x,
                                        uint32_t offset_y,
                                        uint32_t canvas_w,
                                        uint32_t canvas_h,
                                        uint32_t width,
                                        uint32_t height,
                                        double scale,
                                        std::vector<double> scales,
                                        bool verbose,
                                        const data::settings &settings) {
  return data->engine.render(thread_num,
                             job_num,
                             chunk_num,
                             metrics,
                             bg_color,
                             shapes,
                             view_x,
                             view_y,
                             offset_x,
                             offset_y,
                             canvas_w,
                             canvas_h,
                             width,
                             height,
                             scale,
                             scales,
                             verbose,
                             settings);
}

void rendering_engine_wrapper::write_image(image &bmp, int width, int height, const std::string &filename) {
  return data->engine.write_image(bmp, width, height, filename);
}
