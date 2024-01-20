/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>

#include "data/bounding_box.hpp"
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

  void copy_from(const image &other, bounding_box *box_in = nullptr) {
    // #define DEBUG_BOX
    if (box_in) {
      bounding_box &box = *box_in;
      for (size_t y = box.top_left.y; y < box.bottom_right.y; y++) {
        size_t offset_y = y * width;
        for (size_t x = box.top_left.x; x < box.bottom_right.x; x++) {
          size_t offset = offset_y + x;
#ifdef DEBUG_BOX
          if (y == box.top_left.y || x == box.top_left.x || y == box.bottom_right.y - 1 ||
              x == box.bottom_right.x - 1) {
            pixels_[offset].r = 1;
            pixels_[offset].g = 1;
            pixels_[offset].b = 1;
            pixels_[offset].a = 1;
            continue;
          }
#endif
          pixels_[offset] = other.pixels_[offset];
        }
      }
      return;
    }
    if (width == other.width && height == other.height) {
      pixels_.assign(other.pixels_.begin(), other.pixels_.end());
    }
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
#ifdef DEBUG
    if (pixels_.size() <= (y * width) + x) {
      logger(INFO) << "bmp.set(" << x << "," << y << ") exceeds vector length: " << pixels_.size() << std::endl;
      abort();
    }
#endif
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