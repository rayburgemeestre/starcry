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
    auto &bmp_prev = bitmap_back.get(width, height);
    auto &bmp_temp = bitmap_temp.get(width, height);

    // this lock is no longer needed since we got rid of all dependencies
    // std::unique_lock<std::mutex> lock(m);
    bmp.clear_to_color(bg_color);
    bmp_prev.clear_to_color(bg_color);
    bmp_temp.clear_to_color(bg_color);

    // debug font
    // if (!font) {
    //   font = std::make_unique<memory_font>(memory_font::fonts::monaco, 14, 0);
    //   if (!font) {
    //     fprintf(stderr, "Could not load monaco ttf font.\n");
    //     return;
    //   }
    // }

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

        if (shape.type == data::shape_type::circle) {
          /////////////
          // draw_logic_.flag(true);
          // std::swap(bmp, bmp_temp);
          /////////////

          // first one
          // double opacity = 1.0 / double(shape.indexes.size());
          double opacity = 1.0;
          if (shape.indexes.size() > 1) {
            opacity /= shape.indexes.size();
          }
          const auto scale = scales[scales.size() - 1] * scale_ratio;
          draw_logic_.scale(scale);
          auto box = draw_logic_.render_circle(bmp, bmp_prev, shape, opacity, settings);
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
              box = draw_logic_.render_circle(bmp, bmp_prev, shape, opacity, settings);
            } else {
              box.update(draw_logic_.render_circle(bmp, bmp_prev, shape, opacity, settings));
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
            box.update(draw_logic_.render_circle(bmp, bmp_prev, shape, opacity, settings));
            const auto [warp_x, warp_y, warp_view_x, warp_view_y] = warp_data(box, shape);
            if (warp_view_x && warp_x) draw_warped(shape, scale, view_x, warp_view_x, box_x, view_x, view_y, true);
            if (warp_view_y && warp_y) draw_warped(shape, scale, view_y, warp_view_y, box_y, view_x, view_y, true);
          }
          // TODO: comment out entire blug to find some bugs in blending_types.js
          box.normalize(width, height);
          box_x.normalize(width, height);
          box_y.normalize(width, height);
          /////////////
          // draw_logic_.flag(false);
          // std::swap(bmp, bmp_temp);
          /////////////
          //  for (size_t y = box.top_left.y; y < box.bottom_right.y; y++) {
          //    size_t offset_y = y * width;
          //    for (size_t x = box.top_left.x; x < box.bottom_right.x; x++) {
          //      size_t offset = offset_y + x;
          //      // something with alpha going wrong..
          //      draw_logic_.blend_the_pixel(bmp_prev, bmp_prev, shape, x, y, 1., bmp_temp.pixels()[offset]);
          //      // bmp_prev.pixels()[offset] = bmp_temp.pixels()[offset];
          //    }
          //  }
          bmp_prev.copy_from(bmp, &box);
          bmp_prev.copy_from(bmp, &box_x);
          bmp_prev.copy_from(bmp, &box_y);
          // TODO: perhaps not necessary
          std::swap(bmp, bmp_prev);
        } else if (shape.type == data::shape_type::line) {
          // first one
          double opacity = 1.0;
          if (shape.indexes.size() > 1) {
            opacity /= shape.indexes.size();
          }
          draw_logic_.scale(scales[scales.size() - 1] * scale_ratio);  // TODO: fix this
          draw_logic_.render_line(bmp, bmp_prev, shape, opacity, settings);

          // the rest...
          for (const auto &index_data : shape.indexes) {
            const auto &step = index_data.first;
            const auto &index = index_data.second;
            const auto &shape = shapes[step][index];
            draw_logic_.scale(scales[step] * scale_ratio);  // TODO: fix this
            draw_logic_.render_line(bmp, bmp_prev, shape, opacity, settings);
          }
        } else if (shape.type == data::shape_type::text) {
          // first one
          double opacity = 1.0;
          if (shape.indexes.size() > 1) {
            opacity /= shape.indexes.size();
          }
          draw_logic_.scale(scales[scales.size() - 1] * scale_ratio);  // TODO: fix this
          auto box = draw_logic_.render_text(bmp, bmp_prev, shape, opacity, settings);

          // the rest...
          for (const auto &index_data : shape.indexes) {
            const auto &step = index_data.first;
            const auto &index = index_data.second;
            const auto &shape = shapes[step][index];
            draw_logic_.scale(scales[step] * scale_ratio);  // TODO: fix this
            box.update(draw_logic_.render_text(bmp, bmp_prev, shape, opacity, settings));
          }
          box.normalize(width, height);
          bmp_prev.copy_from(bmp, &box);
          std::swap(bmp, bmp_prev);
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
    //    // test
    //    double x = 1.;
    //    double y = 0.;
    //    auto copy = bg_color;
    //    copy.r = 1.;
    //    // bmp.clear_to_color(copy);
    //    for (int i=0; i<1; i++) {
    //      for (int j=0; j<100; j++)
    //        bmp.set(i, j, x, y, y, y);
    //    }

    // if (false) {  // TODO: debug chunks flag
    //   for (uint32_t y = 0; y < canvas_h; y++) al_put_pixel(0, y, al_map_rgba(0, 255, 0, 255));
    //   for (uint32_t x = 0; x < canvas_w; x++) al_put_pixel(x, 0, al_map_rgba(255, 0, 0, 255));
    //   stringstream ss;
    //   ss << "chunk - " << offset_x << ", " << offset_y << endl;
    //   draw_logic_.scale(1);
    //   draw_logic_.offset(0, 0);
    //   draw_logic_.center(width / 2, height / 2);
    //   draw_logic_.render_text(0, 0, 14, ss.str().c_str(), "left");
    // }
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
  bitmap_wrapper bitmap_back;
  bitmap_wrapper bitmap_temp;
};
