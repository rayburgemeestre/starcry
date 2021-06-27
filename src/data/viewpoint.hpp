/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <cstring>

namespace data {

struct viewpoint {
  double scale = 1.;
  double offset_x = 0;
  double offset_y = 0;
  bool raw = false;
  bool preview = false;
  bool save = false;

  template <class Archive>
  void serialize(Archive &ar) {
    ar(scale, offset_x, offset_y, raw, preview, save);
  }
};

inline bool operator==(const viewpoint &lhs, const viewpoint &rhs) {
  return 0 ==
         std::memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(viewpoint));
}

}  // namespace data
