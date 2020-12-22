/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// #define DEBUGMODE

#include "util/progress_visualizer.h"
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

class rendering_engine {
public:
  void initialize() {}

  void render(image &bmp,
              const data::color &bg_color,
              const std::vector<std::vector<data::shape>> &shapes,
              uint32_t offset_x,
              uint32_t offset_y,
              uint32_t canvas_w,
              uint32_t canvas_h,
              uint32_t width,
              uint32_t height,
              double scale) {
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

    draw_logic_.scale(scale);
    draw_logic_.width(width);
    draw_logic_.height(height);
    draw_logic_.center(canvas_w / 2, canvas_h / 2);
    draw_logic_.offset(offset_x, offset_y);

    visualizer_.set_max_frames(shapes[shapes.size() - 1].size());

    if (!shapes.empty()) {
      double index = 0;
      for (const auto &shape : shapes[shapes.size() - 1]) {
        // TODO: make dependend on log_level flag.
        // visualizer_.display(index++);

        if (shape.type == data::shape_type::circle) {
          // first one
          double opacity = 1.0;
          if (shape.indexes.size() > 0) {
            opacity /= (shape.indexes.size() + 1);
          }
          draw_logic_.render_circle<double>(bmp,
                                            shape.time,
                                            shape.x,
                                            shape.y,
                                            shape.radius,
                                            shape.radius_size,
                                            shape.gradients_,
                                            shape.textures,
                                            shape.blending_,
                                            opacity,
                                            shape.opacity,
                                            shape.seed,
                                            shape.scale);

          // the rest...
          for (const auto &index_data : shape.indexes) {
            const auto &step = index_data.first;
            const auto &index = index_data.second;
            const auto &shape = shapes[step][index];
            draw_logic_.render_circle<double>(bmp,
                                              shape.time,
                                              shape.x,
                                              shape.y,
                                              shape.radius,
                                              shape.radius_size,
                                              shape.gradients_,
                                              shape.textures,
                                              shape.blending_,
                                              opacity,
                                              shape.opacity,
                                              shape.seed,
                                              shape.scale);
          }

        } else if (shape.type == data::shape_type::line) {
          // first one
          double opacity = 1.0;
          if (shape.indexes.size() > 0) {
            opacity /= (shape.indexes.size() + 1);
          }
          draw_logic_.render_line<double>(bmp,
                                          shape.time,
                                          shape.x,
                                          shape.y,
                                          shape.x2,
                                          shape.y2,
                                          shape.radius_size,
                                          shape.gradients_,
                                          shape.textures,
                                          shape.blending_,
                                          opacity,
                                          shape.opacity,
                                          shape.seed,
                                          shape.scale);

          // the rest...
          for (const auto &index_data : shape.indexes) {
            const auto &step = index_data.first;
            const auto &index = index_data.second;
            const auto &shape = shapes[step][index];
            draw_logic_.render_line<double>(bmp,
                                            shape.time,
                                            shape.x,
                                            shape.y,
                                            shape.x2,
                                            shape.y2,
                                            shape.radius_size,
                                            shape.gradients_,
                                            shape.textures,
                                            shape.blending_,
                                            opacity,
                                            shape.opacity,
                                            shape.seed,
                                            shape.scale);
          }
        } else if (shape.type == data::shape_type::text) {
          draw_logic_.render_text<double>(shape.x, shape.y, shape.text_size, shape.text, shape.align);
        }
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
    //   draw_logic_.render_text<double>(0, 0, 14, ss.str().c_str(), "left");
    // }
  }

  void write_image(image &bmp, std::string filename) {
    // TODO, see logic for PNG generation
    // std::unique_lock<std::mutex> lock(m);
    // bool ret = al_save_bitmap(filename.c_str(), bmp);
    // if (!ret) throw std::runtime_error("rendering_engine::write_image al_save_bitmap() returned false");
  }

  draw_logic::draw_logic draw_logic_;
  progress_visualizer visualizer_{"Object"};
};
