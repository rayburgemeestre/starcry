/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <cmath>
#include <unordered_map>
#include <vector>

#include "data/color.hpp"
#include "util/math.h"

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

  const buffer_t &buffer() {
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
    for (const auto &col_ : dat.colors) {
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
    buffer_.clear();
    layers_ = 0.;
  }

  void draw_callback(std::function<void(int x, int y, const data::color &)> callback) {
    for (const auto &p : buffer()) {
      // These clamps should be avoided, and in draw_logic we should make sure we don't draw outside bounds!
      // const auto &y = math::clamp(p.first, 0, (int)height);  // TODO: comment
      const auto &y = p.first;  // TODO: comment
      // const auto &y = p.first;
      for (const auto &q : p.second) {
        // const auto &x = math::clamp(q.first, 0, (int)width);  // TODO: comment
        const auto &x = q.first;  // TODO: comment
        // const auto &x = q.first;
        const auto &color_dat = q.second;
        const auto col = get_color(color_dat);
        // TODO: design is suffering a bit, draw_logic_ needs a refactoring
        callback(x, y, col);
      }
    }
  }
};
