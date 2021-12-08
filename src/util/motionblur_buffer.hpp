/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <unordered_map>
#include <vector>

#include "data/color.hpp"

class motionblur_buffer {
private:
public:
  struct color_data {
    std::vector<data::color> colors;
    double counter = 0;
  };

  // Not verified by benchmarking, whether unordered maps are the correct choice
  using buffer_t = std::unordered_map<int, std::unordered_map<int, color_data>>;

  buffer_t buffer_;
  double layers_ = 0.;

  motionblur_buffer() = default;

  const buffer_t& buffer() {
    return buffer_;
  }

  void set_layers(double layers) {
    layers_ = layers;
  }

  void insert(int x, int y, const data::color &color) {
    auto &color_dat = buffer_[y][x];
    color_dat.colors.push_back(color);
    color_dat.counter += 1.0;
    // self updating layers_ only works without chunking, otherwise part of the object that is not moving fast might
    // omit a few layers
    layers_ = std::max(layers_, color_dat.counter);
  }

  bool has(int x, int y) {
    const auto &color_dat = buffer_[y][x];
    return bool(color_dat.counter);
  }

  const data::color get_color(int x, int y) {
    data::color ret;
    const auto dat = buffer_[y][x];
    double divider = 1. / layers_;
    bool first = true;
    if (dat.colors.size() == 1) {
      return dat.colors[0];
    }
    for (const auto &col : dat.colors) {
      if (first) {
        ret = col;
        ret.a *= divider;
        first = false;
        continue;
      }
      ret = blend_add(ret, data::color{col.r, col.g, col.b, col.a * divider});
    }
    ret.normalize();
    return ret;
  }

  const data::color get_color(const color_data &dat) {
    data::color ret;
    double divider = 1. / (std::max(layers_, 1.));
    bool first = true;
    for (const auto &col : dat.colors) {
      if (first) {
        ret = col;
        ret.a *= divider;
        first = false;
        continue;
      }
      ret = blend_add(ret, data::color{col.r, col.g, col.b, col.a * divider});
      ret.normalize();
    }
    return ret;
  }

  void clear() {
    buffer_.clear();
    layers_ = 0.;
  }
};
