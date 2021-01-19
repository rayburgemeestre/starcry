/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "image.hpp"

class bitmap_wrapper {
private:
  image bitmap;
  int bitmap_w = 0;
  int bitmap_h = 0;

public:
  bitmap_wrapper() = default;
  ~bitmap_wrapper() = default;

  // disable copy and move
  bitmap_wrapper(const bitmap_wrapper&) = delete;
  void operator=(const bitmap_wrapper&) = delete;
  bitmap_wrapper(const bitmap_wrapper&&) = delete;
  void operator=(const bitmap_wrapper&&) = delete;

  image& get(int width, int height) {
    if (width != bitmap_w || height != bitmap_h) {
      bitmap.resize(width, height);
      bitmap_w = width;
      bitmap_h = height;
    }
    return bitmap;
  }
};
