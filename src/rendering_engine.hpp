/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// #define DEBUGMODE

#include "bitmap_wrapper.hpp"
#ifndef EMSCRIPTEN
#include "util/image_utils.h"
#endif
#include "util/scope_exit.hpp"

#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
using namespace std;

static std::mutex m;

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

class rendering_engine {
public:
  void initialize() {}

  image &render(size_t thread_num,
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
    auto &bmp = bitmap.get(width, height);

    bmp.clear_to_color(bg_color);

    double scale_ratio = (scale / 1080.) * std::min(canvas_w, canvas_h);

    draw_logic_.scale(scale * scale_ratio);  // TODO: deprecate
    draw_logic_.width(width);
    draw_logic_.height(height);
    draw_logic_.center(canvas_w / 2 - (view_x * scale), canvas_h / 2 - (view_y * scale));
    draw_logic_.offset(offset_x, offset_y);

    if (shapes.empty()) return bmp;

#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
    metrics->resize_job_objects(thread_num, job_num, chunk_num, shapes[shapes.size() - 1].size());
#endif
#endif

    if (!shapes.empty()) {
      double index = 0;
      for (const auto &shape : shapes[shapes.size() - 1]) {
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
        metrics->set_render_job_object_state(thread_num, job_num, chunk_num, index, metrics::job_state::rendering);
#endif
#endif

        if (true && !scales.empty()) {
          draw_logic_.capture_pixels(true);

          // first one
          double opacity = 1.0;
          const auto scale = scales[scales.size() - 1] * scale_ratio;
          draw_logic_.scale(scale);
          bounding_box box;
          switch (shape.type) {
            case data::shape_type::circle:
              box = draw_logic_.render_circle(bmp, shape, opacity, settings);
              break;
            case data::shape_type::line:
              draw_logic_.render_line(bmp, shape, opacity, settings);
              break;
            case data::shape_type::text:
              box = draw_logic_.render_text(bmp, shape, opacity, settings);
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
          const auto draw_warped = [&, this](const auto &shape,
                                             const auto &scale,
                                             auto &warp_view,
                                             const auto &warp_value,
                                             auto &box,
                                             const auto &view_x,
                                             const auto &view_y,
                                             bool update = false) {
            warp_view += double(warp_value);
            draw_logic_.center(canvas_w / 2 - (view_x * scale), canvas_h / 2 - (view_y * scale));
            if (!update) {
              switch (shape.type) {
                case data::shape_type::circle:
                  box = draw_logic_.render_circle(bmp, shape, opacity, settings);
                  break;
                case data::shape_type::line:
                  draw_logic_.render_line(bmp, shape, opacity, settings);
                  break;
                case data::shape_type::text:
                  box = draw_logic_.render_text(bmp, shape, opacity, settings);
                  break;
              }
            } else {
              switch (shape.type) {
                case data::shape_type::circle:
                  box.update(draw_logic_.render_circle(bmp, shape, opacity, settings));
                  break;
                case data::shape_type::line:
                  draw_logic_.render_line(bmp, shape, opacity, settings);
                  break;
                case data::shape_type::text:
                  box.update(draw_logic_.render_text(bmp, shape, opacity, settings));
                  break;
              }
            }
            warp_view -= double(warp_value);
            draw_logic_.center(canvas_w / 2 - (view_x * scale), canvas_h / 2 - (view_y * scale));
          };
          if (warp_view_x && warp_x) draw_warped(shape, scale, view_x, warp_view_x, box_x, view_x, view_y);
          if (warp_view_y && warp_y) draw_warped(shape, scale, view_y, warp_view_y, box_y, view_x, view_y);
          // the rest...
          for (const auto &index_data : shape.indexes) {
            const auto &step = index_data.first;
            const auto &index = index_data.second;
            const auto &shape = shapes[step][index];
            const auto scale = scales[step] * scale_ratio;
            draw_logic_.scale(scale);
            switch (shape.type) {
              case data::shape_type::circle:
                box.update(draw_logic_.render_circle(bmp, shape, opacity, settings));
                break;
              case data::shape_type::line:
                draw_logic_.render_line(bmp, shape, opacity, settings);
                break;
              case data::shape_type::text:
                box.update(draw_logic_.render_text(bmp, shape, opacity, settings));
                break;
            }
            if (warp_view_x && warp_x) draw_warped(shape, scale, view_x, warp_view_x, box_x, view_x, view_y, true);
            if (warp_view_y && warp_y) draw_warped(shape, scale, view_y, warp_view_y, box_y, view_x, view_y, true);
          }
          // TODO: comment out entire blug to find some bugs in blending_types.js
          box.normalize(width, height);
          box_x.normalize(width, height);
          box_y.normalize(width, height);

          draw_logic_.capture_pixels(false);

          auto &ref = draw_logic_.motionblur_buf();
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
              draw_logic_.blend_the_pixel(bmp, shape, x, y, col.a, col);
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

  void write_image(image &bmp, int width, int height, std::string filename) {
#ifndef EMSCRIPTEN
    png::image<png::rgba_pixel> image(width, height);
    copy_to_png(bmp.pixels(), width, height, image, false);
    image.write(filename);
#endif
  }

  draw_logic::draw_logic draw_logic_;
  bitmap_wrapper bitmap;
};
