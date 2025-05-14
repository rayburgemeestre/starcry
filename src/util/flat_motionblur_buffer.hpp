/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include "data/color.hpp"
#include "util/logger.h"

class flat_motionblur_buffer {
public:
  struct color_data {
    static constexpr size_t INITIAL_CAPACITY = 300 * 200;
    std::vector<data::color> colors;
    double counter = 0;

    color_data() {
      colors.reserve(INITIAL_CAPACITY);
    }
  };

private:
  // flat array instead of nested maps for much faster access
  std::vector<color_data> buffer_;

  // track active pixels for efficient iteration
  std::vector<std::pair<int, int>> active_pixels_;

  size_t width_ = 0;
  size_t height_ = 0;
  size_t capacity_width_ = 0;
  size_t capacity_height_ = 0;
  double layers_ = 0.;

  // get index into flat array from 2D coordinates
  inline size_t index(int x, int y) const {
    return y * capacity_width_ + x;
  }

public:
  flat_motionblur_buffer() = default;

  flat_motionblur_buffer(size_t width, size_t height) {
    resize(width, height);
  }

  // resize the buffer to accommodate new dimensions
  void resize(size_t width, size_t height) {
    if (width > capacity_width_ || height > capacity_height_) {
      logger(DEBUG) << "resizing buffer to " << width << "x" << height << std::endl;
      // add some padding to reduce frequency of reallocations
      size_t new_capacity_width = width * 2.0;
      size_t new_capacity_height = height * 2.0;

      // create a new buffer with the new capacity
      std::vector<color_data> new_buffer(new_capacity_width * new_capacity_height);

      // copy existing data to the new buffer
      for (const auto& [x, y] : active_pixels_) {
        // use direct index calculations to avoid issues with changing capacity
        size_t old_idx = y * capacity_width_ + x;
        size_t new_idx = y * new_capacity_width + x;

        // move the data to the new buffer
        new_buffer[new_idx] = std::move(buffer_[old_idx]);
      }

      // update capacity and swap buffers
      capacity_width_ = new_capacity_width;
      capacity_height_ = new_capacity_height;
      buffer_.swap(new_buffer);

      active_pixels_.reserve(width * height / 4);
    }

    // update current dimensions
    width_ = width;
    height_ = height;
  }

  const std::vector<std::pair<int, int>>& active_pixels() const {
    return active_pixels_;
  }

  size_t width() const {
    return width_;
  }
  size_t height() const {
    return height_;
  }

  void set_layers(double layers) {
    layers_ = layers;
  }

  void insert(int x, int y, const data::color& color) {
    // ensure buffer is large enough
    if (x >= static_cast<int>(width_) || y >= static_cast<int>(height_)) {
      resize(std::max(width_, static_cast<size_t>(x + 1)), std::max(height_, static_cast<size_t>(y + 1)));
    }

    size_t idx = index(x, y);
    auto& color_dat = buffer_[idx];

    // if this is the first color at this pixel, add to active pixels list
    // this way we can use a std::vector instead of a slower std::set
    if (color_dat.counter == 0) {
      active_pixels_.emplace_back(x, y);
    }

    color_dat.colors.push_back(color);
    color_dat.counter += 1.0;
    layers_ = std::max(layers_, color_dat.counter);
  }

  bool has(int x, int y) const {
    return bool(buffer_[index(x, y)].counter);
  }

  const color_data& get_data(int x, int y) const {
    return buffer_[index(x, y)];
  }

  const data::color get_color(const color_data& dat) {
    data::color ret;
    double divider = 1. / (std::max(layers_, 1.));
    bool first = true;
    for (const auto& col_ : dat.colors) {
      auto col = col_;
      // TODO: got some weird artifacts when I was dealing with colors with alpha of zero..
      // this is a workaround, probably in the wrong place
      col.a = std::max(std::numeric_limits<double>::epsilon(), col.a);
      if (first) {
        ret = col;
        ret.a *= ret.a;
        ret.a *= divider;
        first = false;
        continue;
      }
      ret = blend(ret, data::color{col.r, col.g, col.b, col.a * col.a * divider});
    }
    ret.a = sqrt(ret.a);
    ret.normalize();
    return ret;
  }

  void clear() {
    // only reset counters and clear color vectors, but keep memory allocated
    for (const auto& [x, y] : active_pixels_) {
      size_t idx = index(x, y);
      buffer_[idx].colors.clear();
      buffer_[idx].counter = 0;
    }
    // clear active pixels tracking
    active_pixels_.clear();
    layers_ = 0.0;
  }
};
