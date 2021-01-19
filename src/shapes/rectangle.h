/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "shapes/position.h"

class rectangle {
public:
  const position top_left;
  const position bottom_right;
  const double width;
  const double height;

  rectangle(const position&& top_left, const position&& bottom_right);
  rectangle(position&& top_left, double width, double height);

  bool contains(const position& point);
  bool overlaps(const rectangle& other);
};
