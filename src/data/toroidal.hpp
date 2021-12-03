/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "cereal/cereal.hpp"

namespace data {

struct toroidal {
  int width;
  int height;

  toroidal() {}

  template <class Archive>
  void serialize(Archive &ar) {
    ar(width, height);
  }
};

inline bool operator==(const toroidal &lhs, const toroidal &rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(toroidal));
}

}  // namespace data
