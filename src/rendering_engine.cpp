/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmath>
#include <cstring>
#include <functional>
#include <mutex>

#include "bitmap_wrapper.hpp"
#include "rendering_engine.h"

#ifndef EMSCRIPTEN
#include <fmt/core.h>
#include "util/image_utils.h"
#endif
#include "util/scope_exit.hpp"

using namespace std;

template <typename T>
constexpr T squared(T num) {
  return (num * num);
}
template <typename T>
constexpr T squared_dist(T num, T num2) {
  return (num - num2) * (num - num2);
}

template <typename T>
constexpr T distance(T x1, T x2, T y1, T y2) {
  return sqrt(squared_dist<T>(x1, x2) + squared_dist<T>(y1, y2));
}

template <typename T, typename double_type>
constexpr int half_chord_length(T radiusOuterCircle, T relativeY) {
  return static_cast<int>(sqrt(static_cast<double_type>(squared(radiusOuterCircle) - squared(relativeY))) + 0.5);
}
template <typename double_type>
inline int round_to_int(double_type in) {
  return static_cast<int>(0.5 + in);
}

// TODO: the fact that below dependencies depend on above functions needs to be fixed

#include "data/shape.hpp"
#include "draw_logic.hpp"
#include "image.hpp"
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
#include "starcry/metrics.h"
#endif
#endif

image &rendering_engine::render(size_t thread_num,
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
                                double debug) {
  if (!draw_logic_) {
    draw_logic_ = std::make_shared<draw_logic::draw_logic>();
  }
  if (!bitmap) {
    bitmap = std::make_shared<bitmap_wrapper>();
  }
  auto &bmp = bitmap->get(width, height);

  bmp.clear_to_color(bg_color);

  double scale_ratio = (top_scale / 1080.) * canvas_h;  // std::min(canvas_w, canvas_h);

  auto drawlogic = *draw_logic_;

  drawlogic.scale(top_scale /* * scale_ratio */);
  drawlogic.canvas_width(canvas_w);
  drawlogic.canvas_height(canvas_h);
  drawlogic.height(height);
  drawlogic.width(width);
  drawlogic.height(height);
  drawlogic.center(view_x, view_y);
  drawlogic.offset(offset_x, offset_y);

  if (shapes.empty()) return bmp;

#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
  metrics->resize_job_objects(thread_num, job_num, chunk_num, shapes[shapes.size() - 1].size());
#endif
#endif

  if (debug) {
    double r = double(offset_y) / double(canvas_h), g = 0, b = 0, a = 1;
    for (auto i = 0; i < width; i++) {
      bmp.set(i, 0, r, g, b, a);
    }
    for (auto i = 0; i < height; i++) {
      bmp.set(width / 2, i, r, g, b, a);
    }
#ifndef EMSCRIPTEN  // TODO: fix for emscripten
    auto fun = [&]() {
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
        shape.blending_ = data::blending_type::normal;
        return shape;
      };

      drawlogic.render_text(bmp, ct(0, fmt::format("canvas: ({}, {})", canvas_w, canvas_h)), 1., settings, true);
      drawlogic.render_text(bmp, ct(20, fmt::format("size: ({}, {})", width, height)), 1., settings, true);
      drawlogic.render_text(bmp, ct(60, fmt::format("offset: ({}, {})", offset_x, offset_y)), 1., settings, true);
      drawlogic.render_text(bmp, ct(80, fmt::format("view: ({}, {})", view_x, view_y)), 1., settings, true);
      drawlogic.render_text(bmp, ct(100, fmt::format("scale: {}", top_scale)), 1., settings, true);
      drawlogic.render_text(bmp, ct(120, fmt::format("scale ratio: {}", scale_ratio)), 1., settings, true);
    };
    scope_exit<decltype(fun)> se(fun);
#endif
  }

  if (!shapes.empty()) {
    double index = 0;
    for (const auto &shape : shapes[shapes.size() - 1]) {
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
      metrics->set_render_job_object_state(thread_num, job_num, chunk_num, index, metrics::job_state::rendering);
#endif
#endif

      if (true && !scales.empty()) {
        drawlogic.capture_pixels(true);

        // first one
        double opacity = 1.0;
        const auto scale = scales[scales.size() - 1] /* * scale_ratio */;
        drawlogic.scale(top_scale * scale);
        bounding_box box;
        switch (shape.type) {
          case data::shape_type::circle:
            box = drawlogic.render_circle(bmp, shape, opacity, settings);
            break;
          case data::shape_type::line:
            drawlogic.render_line(bmp, shape, opacity, settings);
            break;
          case data::shape_type::text:
            box = drawlogic.render_text(bmp, shape, opacity, settings);
            break;
          case data::shape_type::none:
            break;
        }

        bounding_box box_x;
        bounding_box box_y;
        const auto warp_data = [&](const auto &box, const auto &shape) {
          // zero is potential future pivot (e.g., from parent)
          const auto [a, b, c, d] = std::make_tuple(canvas_w / 2. + 0. - shape.warp_width / 2.,
                                                    canvas_w / 2. + 0. + shape.warp_width / 2.,
                                                    canvas_h / 2. + 0. - shape.warp_height / 2.,
                                                    canvas_h / 2. + 0. + shape.warp_height / 2.);
          return std::make_tuple(box.top_left.x <= a || box.bottom_right.x >= b,
                                 box.top_left.y <= c || box.bottom_right.y >= d,
                                 (box.top_left.x <= a) ? -shape.warp_width : shape.warp_width,
                                 (box.top_left.y <= c) ? -shape.warp_height : shape.warp_height);
        };
        const auto [warp_x, warp_y, warp_view_x, warp_view_y] = warp_data(box, shape);
        const auto draw_warped = [&](const auto &shape,
                                     const auto &scale,
                                     auto &warp_view,
                                     const auto &warp_value,
                                     auto &box,
                                     const auto &view_x,
                                     const auto &view_y,
                                     bool update = false) {
          warp_view += double(warp_value);
          drawlogic.center(canvas_w / 2 - (view_x * scale), canvas_h / 2 - (view_y * scale));
          if (!update) {
            switch (shape.type) {
              case data::shape_type::circle:
                box = drawlogic.render_circle(bmp, shape, opacity, settings);
                break;
              case data::shape_type::line:
                drawlogic.render_line(bmp, shape, opacity, settings);
                break;
              case data::shape_type::text:
                box = drawlogic.render_text(bmp, shape, opacity, settings);
                break;
              case data::shape_type::none:
                break;
            }
          } else {
            switch (shape.type) {
              case data::shape_type::circle:
                box.update(drawlogic.render_circle(bmp, shape, opacity, settings));
                break;
              case data::shape_type::line:
                drawlogic.render_line(bmp, shape, opacity, settings);
                break;
              case data::shape_type::text:
                box.update(drawlogic.render_text(bmp, shape, opacity, settings));
                break;
              case data::shape_type::none:
                break;
            }
          }
          warp_view -= double(warp_value);
          drawlogic.center(canvas_w / 2 - (view_x * scale), canvas_h / 2 - (view_y * scale));
        };
        if (warp_view_x && warp_x) draw_warped(shape, scale, view_x, warp_view_x, box_x, view_x, view_y);
        if (warp_view_y && warp_y) draw_warped(shape, scale, view_y, warp_view_y, box_y, view_x, view_y);
        // the rest...
        for (const auto &index_data : shape.indexes) {
          const auto &step = index_data.first;
          const auto &index = index_data.second;
          const auto &shape = shapes[step][index];
          const auto scale = scales[step] /* * scale_ratio */;
          drawlogic.scale(top_scale * scale);
          switch (shape.type) {
            case data::shape_type::circle:
              box.update(drawlogic.render_circle(bmp, shape, opacity, settings));
              break;
            case data::shape_type::line:
              drawlogic.render_line(bmp, shape, opacity, settings);
              break;
            case data::shape_type::text:
              box.update(drawlogic.render_text(bmp, shape, opacity, settings));
              break;
            case data::shape_type::none:
              break;
          }
          if (warp_view_x && warp_x) draw_warped(shape, scale, view_x, warp_view_x, box_x, view_x, view_y, true);
          if (warp_view_y && warp_y) draw_warped(shape, scale, view_y, warp_view_y, box_y, view_x, view_y, true);
        }
        // TODO: comment out entire blug to find some bugs in blending_types.js
        box.normalize(width, height);
        box_x.normalize(width, height);
        box_y.normalize(width, height);

        drawlogic.capture_pixels(false);

        auto &ref = drawlogic.motionblur_buf();
        ref.set_layers(shape.indexes.size());
        for (const auto &p : ref.buffer()) {
          // These clamps should be avoided, and in draw_logic we should make sure we don't draw outside bounds!
          const auto &y = math::clamp(p.first, 0, (int)height);  // TODO: comment
          // const auto &y = p.first;
          for (const auto &q : p.second) {
            const auto &x = math::clamp(q.first, 0, (int)width);  // TODO: comment
            // const auto &x = q.first;
            const auto &color_dat = q.second;
            const auto col = ref.get_color(color_dat);
            // TODO: design is suffering a bit, draw_logic_ needs a refactoring
            drawlogic.blend_the_pixel(bmp, shape, x, y, col.a, col);
          }
        }
      }
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
      metrics->set_render_job_object_state(thread_num, job_num, chunk_num, index, metrics::job_state::rendered);
#endif
#endif
      index++;
    }
    return bmp;
  }
  return bmp;
}

void rendering_engine::write_image(image &bmp, int width, int height, std::string filename) {
#ifndef EMSCRIPTEN
  png::image<png::rgba_pixel> image(width, height);
  copy_to_png(bmp.pixels(), width, height, image, false);
  image.write(filename);
#endif
}
