/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "image.hpp"

struct rendering_engine_wrapper_class_data;

namespace data {
struct color;
struct shape;
struct settings;
}  // namespace data

class metrics;

class rendering_engine_wrapper {
public:
  rendering_engine_wrapper();

  // disable copy and move
  rendering_engine_wrapper(const rendering_engine_wrapper &) = delete;
  void operator=(const rendering_engine_wrapper &) = delete;
  rendering_engine_wrapper(const rendering_engine_wrapper &&) = delete;
  void operator=(const rendering_engine_wrapper &&) = delete;

  void initialize();

  void render(size_t thread_num,
              size_t job_num,
              size_t chunk_num,
              std::shared_ptr<metrics> &metrics,
              image &bmp,
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
              const data::settings &settings);

  void write_image(image &bmp, const std::string &filename);

  std::shared_ptr<rendering_engine_wrapper_class_data> data;
};
