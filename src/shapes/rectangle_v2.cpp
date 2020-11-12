/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "shapes/rectangle_v2.h"

rectangle_v2::rectangle_v2(const position&& top_left, const position&& bottom_right)
    : top_left(top_left),
      bottom_right(bottom_right),
      width(bottom_right.x - top_left.x),
      height(bottom_right.y - top_left.y) {}
rectangle_v2::rectangle_v2(position&& top_left, double width, double height)
    : top_left(top_left), bottom_right(top_left.x + width, top_left.y + height), width(width), height(height) {}

bool rectangle_v2::contains(const position& point) {
  return (point.x >= top_left.x && point.x < bottom_right.x && point.y >= top_left.y && point.y < bottom_right.y);
}
