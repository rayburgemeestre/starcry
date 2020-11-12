/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "shapes/circle_v2.h"

#include <algorithm>
#include <cmath>

circle_v2::circle_v2(position pos, double radius, double radiussize)
    : pos(pos), radius(radius), radiussize(radiussize) {}

bool circle_v2::contains(const position& point) const {
  const auto d = pow((point.x - pos.x), 2.0) + pow((point.y - pos.y), 2.0);
  return d <= pow((radius * 2 + radiussize * 2), 2.0);
}

// source https://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection/1879223#1879223
bool circle_v2::intersects(const rectangle_v2& range) const {
  // Find the closest point to the circle within the rectangle
  double closestX = std::clamp(pos.x, range.top_left.x, range.bottom_right.x);
  double closestY = std::clamp(pos.y, range.top_left.y, range.bottom_right.y);

  // Calculate the distance between the circle's center and this closest point
  double distanceX = pos.x - closestX;
  double distanceY = pos.y - closestY;

  // If the distance is less than the circle's radius, an intersection occurs
  double distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
  auto p = ((radius + radiussize) * (radius + radiussize));
  return distanceSquared < p;
}
