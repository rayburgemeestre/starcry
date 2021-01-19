/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "shapes/circle.h"

#include <cstddef>
#include <memory>
#include <vector>

class point_type {
public:
  position pos;
  size_t userdata;
  point_type(position pos, int userdata);
};

class quadtree {
public:
  rectangle boundary;
  size_t capacity;
  bool divided;
  std::vector<point_type> points;

  std::unique_ptr<quadtree> northwest;
  std::unique_ptr<quadtree> northeast;
  std::unique_ptr<quadtree> southeast;
  std::unique_ptr<quadtree> southwest;

  quadtree();
  quadtree(rectangle boundary, size_t capacity);

  void subdivide();
  bool insert(point_type point);
  void query(size_t index, const circle& range, std::vector<point_type>& found);
};
