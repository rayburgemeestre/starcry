/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "data/coord.hpp"

class bounding_box {
public:
  coord top_left;
  coord bottom_right;
  bool initialized;

  bounding_box() : initialized(false) {}
  void update(bounding_box&& other) {
    if (!initialized) {
      top_left = other.top_left;
      bottom_right = other.bottom_right;
      initialized = true;
    } else {
      top_left.x = std::min(other.top_left.x, top_left.x);
      top_left.y = std::min(other.top_left.y, top_left.y);
      bottom_right.x = std::max(other.bottom_right.x, bottom_right.x);
      bottom_right.y = std::max(other.bottom_right.y, bottom_right.y);
    }
  }
  void normalize(int width, int height) {
    if (top_left.x < 0) top_left.x = 0;
    if (top_left.y < 0) top_left.y = 0;
    if (bottom_right.x > width) bottom_right.x = width;
    if (bottom_right.y > height) bottom_right.y = height;
  }

  friend std::ostream& operator<<(std::ostream& stream, const bounding_box& b) {
    stream << "#box{" << b.top_left.x << "," << b.top_left.y << " - " << b.bottom_right.x << "," << b.bottom_right.y
           << "}";
    return stream;
  }
};
