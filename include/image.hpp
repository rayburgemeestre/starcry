/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <iostream>
#include <mutex>

#include "data/color.hpp"

class image {
private:
  std::vector<data::color> pixels_;
  size_t width;
  size_t height;

public:
  image() : image(0, 0) {}
  image(size_t width, size_t height) : pixels_(width * height, {0, 0, 0, 0}), width(width), height(height) {}

  std::vector<data::color> &pixels() {
    return pixels_;
  }

  void resize(int width, int height) {
    this->width = width;
    this->height = height;

    size_t size = width * height;
    if (pixels_.size() > size) {
      pixels_.erase(pixels_.begin() + size, pixels_.end());
    } else if (pixels_.size() < size) {
      std::fill_n(std::back_inserter(pixels_), size - pixels_.size(), data::color{0, 0, 0, 0});
    }
  }

  data::color &get(const int &x, const int &y) {
    return pixels_[(y * width) + x];
  }

  void set(const int &x, const int &y, double &r, double &g, double &b, double &a) {
    auto &p = pixels_[(y * width) + x];
    p.r = r;
    p.g = g;
    p.b = b;
    p.a = a;
  }

  void clear_to_color(const data::color &col) {
    std::fill(pixels_.begin(), pixels_.end(), col);
    // below is much faster, however only works with chars.
    // memset((data::color *)(&(pixels_[0])), 0x00, pixels_.size() * sizeof(data::color) * sizeof(char));
  }
};