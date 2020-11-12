/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "shapes/position.h"
#include "shapes/rectangle_v2.h"

class circle_v2 {
public:
  const position pos;
  const double radius;
  const double radiussize;

  circle_v2(position pos, double radius, double radiussize);

  bool contains(const position& point) const;

  bool intersects(const rectangle_v2& range) const;
};
