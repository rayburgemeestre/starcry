/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// #define DEBUGMODE

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
              const data::settings &settings) {
    // this lock is no longer needed since we got rid of all dependencies
    // std::unique_lock<std::mutex> lock(m);
    bmp.clear_to_color(bg_color);

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
    draw_logic_.center(canvas_w / 2 - view_x, canvas_h / 2 - view_y);
    draw_logic_.offset(offset_x, offset_y);

    if (shapes.empty()) return;

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
          // first one
          double opacity = 1.0;
          if (shape.indexes.size() > 0) {
            opacity /= (shape.indexes.size() + 1);
          }
          draw_logic_.scale(scales[scales.size() - 1] * scale_ratio);
          draw_logic_.render_circle(bmp, shape, opacity, settings);

          // the rest...
          for (const auto &index_data : shape.indexes) {
            const auto &step = index_data.first;
            const auto &index = index_data.second;
            const auto &shape = shapes[step][index];
            draw_logic_.scale(scales[step] * scale_ratio);  // TODO: fix this
            draw_logic_.render_circle(bmp, shape, opacity, settings);
          }

        } else if (shape.type == data::shape_type::line) {
          // first one
          double opacity = 1.0;
          if (shape.indexes.size() > 0) {
            opacity /= (shape.indexes.size() + 1);
          }
          draw_logic_.scale(scales[scales.size() - 1] * scale_ratio);  // TODO: fix this
          draw_logic_.render_line(bmp, shape, opacity, settings);

          // the rest...
          for (const auto &index_data : shape.indexes) {
            const auto &step = index_data.first;
            const auto &index = index_data.second;
            const auto &shape = shapes[step][index];
            draw_logic_.scale(scales[step] * scale_ratio);  // TODO: fix this
            draw_logic_.render_line(bmp, shape, opacity, settings);
          }
        } else if (shape.type == data::shape_type::text) {
          draw_logic_.scale(scales[scales.size() - 1] * scale_ratio);  // TODO: fix this
          draw_logic_.render_text(shape.x, shape.y, shape.text_size, shape.text, shape.align);
        }
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
        metrics->set_render_job_object_state(thread_num, job_num, chunk_num, index, metrics::job_state::rendered);
#endif
#endif
        index++;
      }
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
  }

  void write_image(image &bmp, std::string filename) {
    // TODO, see logic for PNG generation
    // std::unique_lock<std::mutex> lock(m);
    // bool ret = al_save_bitmap(filename.c_str(), bmp);
    // if (!ret) throw std::runtime_error("rendering_engine::write_image al_save_bitmap() returned false");
  }

  draw_logic::draw_logic draw_logic_;
};