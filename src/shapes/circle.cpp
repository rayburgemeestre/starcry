/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "shapes/circle.h"

#include <algorithm>
#include <cmath>

circle::circle(position pos, double radius, double radiussize) : pos(pos), radius(radius), radiussize(radiussize) {}

bool circle::contains(const position& point) const {
  auto rad = radius + radiussize;
  return ((point.x - pos.x) * (point.x - pos.x) + (point.y - pos.y) * (point.y - pos.y) <= rad * rad);
}

// source https://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection/1879223#1879223
bool circle::intersects(const rectangle& range) const {
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

bool circle::overlaps(const circle& other) const {
  double distanceX = pos.x - other.pos.x;
  double distanceY = pos.y - other.pos.y;
  double distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
  auto p = ((radius + radiussize) * (radius + radiussize));
  return distanceSquared < p;
}
