/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace data {

class coord {
public:
  double x = 0;
  double y = 0;

  coord() = default;

  coord(double x, double y) : x(x), y(y) {}
};

}  // namespace data