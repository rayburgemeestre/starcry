/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "rendering_engine/debug.h"

#include "draw_logic.hpp"
#include "image.hpp"
#include "rendering_engine/render_params.hpp"
#include "util/scope_exit.hpp"

void draw_debug(image &target_bmp,
                std::shared_ptr<draw_logic::draw_logic> dl,
                render_params &params,
                uint32_t offset_x,
                uint32_t offset_y,
                uint32_t width,
                uint32_t height,
                double scale_ratio) {
  double r = double(offset_y) / double(params.canvas_h), g = 0, b = 0, a = 1;
  for (auto i = uint32_t(0); i < width; i++) {
    target_bmp.set(i, 0, r, g, b, a);
  }
  for (auto i = uint32_t(0); i < height; i++) {
    target_bmp.set(width / 2, i, r, g, b, a);
  }
  scope_exit se([&]() {
    const auto ct = [&](double offset_y, const std::string &text) {
      data::shape shape;  // = shape;
      shape.x = 0;
      shape.y = offset_y;
      shape.r = 1;
      shape.g = 1;
      shape.b = 1;
      shape.opacity = 1;
      shape.type = data::shape_type::text;
      shape.text = text;
      shape.text_fixed = true;
      shape.text_size = 15;
      shape.align = "left";
      shape.gradient_id_str = "";
      shape.gradients_.emplace_back(1.0, data::gradient{});
      shape.gradients_[0].second.colors.emplace_back(std::make_tuple(0.0, data::color{1.0, 1, 1, 1}));
      shape.gradients_[0].second.colors.emplace_back(std::make_tuple(1.0, data::color{1.0, 1, 1, 1}));
      shape.texture_id_str = "";
      shape.blending_ = data::blending_type::normal;
      return shape;
    };

    double offset_y_counter = 0;
    const auto offset_y = [&]() {
      offset_y_counter += 20;
      return offset_y_counter;
    };

    dl->render_text(target_bmp, ct(offset_y(), fmt::format("!chunk: {}", params.chunk_num)), 1., params.settings, true);
    dl->render_text(target_bmp,
                    ct(offset_y(), fmt::format("canvas: ({}, {})", params.canvas_w, params.canvas_h)),
                    1.,
                    params.settings,
                    true);
    dl->render_text(
        target_bmp, ct(offset_y(), fmt::format("size: ({}, {})", width, height)), 1., params.settings, true);
    dl->render_text(
        target_bmp, ct(offset_y(), fmt::format("offset: ({}, {})", offset_x, offset_y())), 1., params.settings, true);
    dl->render_text(target_bmp,
                    ct(offset_y(), fmt::format("view: ({}, {})", params.view_x, params.view_y)),
                    1.,
                    params.settings,
                    true);
    dl->render_text(target_bmp, ct(offset_y(), fmt::format("scale: {}", params.top_scale)), 1., params.settings, true);
    dl->render_text(target_bmp, ct(offset_y(), fmt::format("scale ratio: {}", scale_ratio)), 1., params.settings, true);
  });
}
