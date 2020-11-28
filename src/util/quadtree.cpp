/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "util/quadtree.h"

point_type::point_type(position pos, int userdata) : pos(pos), userdata(userdata) {}

quadtree::quadtree() : boundary(rectangle_v2(position(0, 0), 0, 0)) {}

quadtree::quadtree(rectangle_v2 boundary, size_t capacity)
    : boundary(boundary), capacity(std::max(capacity, size_t(1))), divided(false) {}

void quadtree::subdivide() {
  auto x = boundary.top_left.x;
  auto y = boundary.top_left.y;
  auto w = boundary.width / 2;
  auto h = boundary.height / 2;

  rectangle_v2 nw(position(x + w, y), w, h);
  rectangle_v2 ne(position(x + w, y + h), w, h);
  rectangle_v2 se(position(x, y + h), w, h);
  rectangle_v2 sw(position(x, y), w, h);

  northwest = std::make_unique<quadtree>(nw, capacity);
  northeast = std::make_unique<quadtree>(ne, capacity);
  southeast = std::make_unique<quadtree>(se, capacity);
  southwest = std::make_unique<quadtree>(sw, capacity);

  divided = true;
}

bool quadtree::insert(point_type point) {
  if (!boundary.contains(point.pos)) {
    return false;
  }
  if (points.size() < capacity) {
    points.push_back(point);
    return true;
  }
  if (!divided) {
    subdivide();
  }
  return (northeast->insert(point) || northwest->insert(point) || southeast->insert(point) || southwest->insert(point));
}

void quadtree::query(size_t index, const circle_v2& range, std::vector<point_type>& found) {
  if (!range.intersects(boundary)) {
    return;
  }
  for (const auto& p : points) {
    if (p.userdata == index) {
      // skip collision with self
      continue;
    }
    if (range.contains(p.pos)) {
      found.push_back(p);
    }
  }
  if (divided) {
    northwest->query(index, range, found);
    northeast->query(index, range, found);
    southwest->query(index, range, found);
    southeast->query(index, range, found);
  }
}
