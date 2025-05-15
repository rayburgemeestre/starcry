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

class motionblur_buffer_flat_rev {
public:
  struct color_data {
    static constexpr size_t INITIAL_CAPACITY = 8;
    std::vector<data::color> colors;
    double counter = 0;

    color_data() {
      colors.reserve(INITIAL_CAPACITY);
    }
  };

  // pixel data container storing location and color data
  struct pixel_data {
    int x;
    int y;
    color_data data;
  };

private:
  // sentinel value for empty pixels in buffer_
  static constexpr size_t EMPTY_PIXEL = static_cast<size_t>(-1);

  // flat array of indices into active_pixels_ or EMPTY_PIXEL for empty pixels
  std::vector<size_t> buffer_;

  // track active pixels and their color data for efficient iteration
  std::vector<pixel_data> active_pixels_;

  // cached pairs of (x,y) coordinates for public interface
  mutable std::vector<std::pair<int, int>> active_pixel_pairs_;
  mutable bool active_pixel_pairs_dirty_ = true;

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
  motionblur_buffer_flat_rev() = default;

  motionblur_buffer_flat_rev(size_t width, size_t height) {
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
      std::vector<size_t> new_buffer(new_capacity_width * new_capacity_height, EMPTY_PIXEL);

      // copy existing data to the new buffer
      for (size_t i = 0; i < active_pixels_.size(); ++i) {
        const auto& entry = active_pixels_[i];
        // use direct index calculations to avoid issues with changing capacity
        size_t old_idx = entry.y * capacity_width_ + entry.x;
        size_t new_idx = entry.y * new_capacity_width + entry.x;

        // store index to active_pixels_ in the new buffer
        new_buffer[new_idx] = i;
      }

      // update capacity and swap buffers
      capacity_width_ = new_capacity_width;
      capacity_height_ = new_capacity_height;
      buffer_.swap(new_buffer);

      active_pixels_.reserve(width * height / 4);
    } else if (buffer_.empty()) {
      // initialize the buffer with EMPTY_PIXEL values
      capacity_width_ = width * 2.0;
      capacity_height_ = height * 2.0;
      buffer_.resize(capacity_width_ * capacity_height_, EMPTY_PIXEL);
    }

    // update current dimensions
    width_ = width;
    height_ = height;
  }

  const std::vector<pixel_data>& active_pixels() const {
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

    // if this is the first color at this pixel, add to active pixels list
    if (buffer_[idx] == EMPTY_PIXEL) {
      active_pixels_.push_back({x, y, color_data()});
      buffer_[idx] = active_pixels_.size() - 1;
      active_pixel_pairs_dirty_ = true;
    }

    auto& color_dat = active_pixels_[buffer_[idx]].data;
    color_dat.colors.push_back(color);
    color_dat.counter += 1.0;
    layers_ = std::max(layers_, color_dat.counter);
  }

  bool has(int x, int y) const {
    size_t idx = index(x, y);
    return buffer_[idx] != EMPTY_PIXEL;
  }

  const color_data& get_data(int x, int y) const {
    size_t idx = index(x, y);
    return active_pixels_[buffer_[idx]].data;
  }

  const data::color get_color(const color_data& dat) {
    data::color ret;
    double divider = 1. / (std::max(layers_, 1.));
    bool first = true;
    for (const auto& col_ : dat.colors) {
      auto col = col_;
      // todo: got some weird artifacts when i was dealing with colors with alpha of zero..
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
    // mark all buffer entries as empty
    for (size_t i = 0; i < active_pixels_.size(); ++i) {
      const auto& entry = active_pixels_[i];
      size_t idx = index(entry.x, entry.y);
      buffer_[idx] = EMPTY_PIXEL;
    }
    // clear active pixels tracking
    active_pixels_.clear();
    active_pixel_pairs_dirty_ = true;
    layers_ = 0.0;
  }

  void draw_callback(std::function<void(const pixel_data&, const data::color&)> callback) {
    // Iterate only through active pixels - much faster than scanning the entire buffer
    for (const auto& data : active_pixels()) {
      // Get the color data and blend it
      const auto col = get_color(data.data);
      callback(data, col);
    }
  }
};
