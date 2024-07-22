/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "data/bounding_box.hpp"
#include "rendering_engine/render_params.hpp"
#include "util/image_splitter.hpp"
#include "util/random.hpp"

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
  image render(render_params &params, uint32_t offset_x, uint32_t offset_y, uint32_t width, uint32_t height);

  std::shared_ptr<draw_logic::draw_logic> draw_logic_;
  std::shared_ptr<bitmap_wrapper> bitmap;
  std::shared_ptr<bitmap_wrapper> cumulative_bitmap;

private:
  void _render(
      image &bmp, render_params &params, uint32_t offset_x, uint32_t offset_y, uint32_t width, uint32_t height);

  util::random_generator rand_;
  void draw_captured_pixels(image &target_bmp, uint32_t width, uint32_t height, const data::shape &shape);
  bounding_box invoke_render(image &target_bmp,
                             render_params &params,
                             const auto &shape,
                             double opacity,
                             const data::settings &settings,
                             double scale,
                             double scale_ratio,
                             double offset_x,
                             double offset_y);
};