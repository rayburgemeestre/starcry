/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace draw_logic {
class draw_logic;
}
namespace data {
struct color;
struct shape;
struct settings;
}  // namespace data
class bitmap_wrapper;
class metrics;
class image;

class rendering_engine {
public:
  image &render(size_t thread_num,
                size_t job_num,
                size_t chunk_num,
                size_t num_chunks,
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
                double top_scale,
                std::vector<double> scales,
                bool verbose,
                const data::settings &settings,
                double debug);

  void write_image(image &bmp, int width, int height, std::string filename);

  std::shared_ptr<draw_logic::draw_logic> draw_logic_;
  std::shared_ptr<bitmap_wrapper> bitmap;
  std::shared_ptr<bitmap_wrapper> cumulative_bitmap;

private:
  image &_render(size_t thread_num,
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
                 double top_scale,
                 std::vector<double> scales,
                 bool verbose,
                 const data::settings &settings,
                 double debug);
};