/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cassert>
#include <mutex>
#include <set>
#include <vector>

#include "data/bounding_box.hpp"
#include "data/color.hpp"
#include "util/logger.h"

class image;

class image_repository {
public:
  static image_repository &instance() {
    static image_repository instance;
    return instance;
  }

  std::set<image *> images_;
  mutable std::mutex mutex_;

  void register_image(image *img) {
    std::unique_lock<std::mutex> lock(mutex_);
    images_.insert(img);
  }

  void unregister_image(image *img) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (images_.contains(img)) {
      images_.erase(img);
    }
  }

  void print();

  image_repository() = default;

public:
  image_repository(const image_repository &) = delete;
  image_repository &operator=(const image_repository &) = delete;
};

class image {
private:
  std::vector<data::color> pixels_;
  size_t width;
  size_t height;

public:
  image() : image(0, 0) {}
  image(size_t width, size_t height) : pixels_(width * height, {0, 0, 0, 0}), width(width), height(height) {
    image_repository::instance().register_image(this);
  }

  image(const image &) = delete;
  void operator=(const image &) = delete;
  image(const image &&) = delete;
  void operator=(const image &&) = delete;

  ~image() {
    image_repository::instance().unregister_image(this);
  }

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
    assert(pixels_.size() == size);
    // pixels_.shrink_to_fit();
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

inline void image_repository::print() {
  size_t total_bytes = 0;
  for (auto *img : images_) {
    total_bytes += (img->pixels().size() * sizeof(data::color));
  }
  logger(DEBUG) << "TOTAL  in GB: " << (total_bytes / 1024. / 1024. / 1024.) << std::endl;
}