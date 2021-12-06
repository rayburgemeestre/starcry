/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
//#include <caf/meta/type_name.hpp>
#include "cereal/types/tuple.hpp"
#include "cereal/types/vector.hpp"

#include "data/color.hpp"

namespace data {

struct gradient {
  std::vector<std::tuple<double, color>> colors;

  gradient() {}

  color get(double index) const {
    // TODO: refactor, this COPY & PASTE from gradient.cpp
    size_t counter = 0;
    double processed_index = 0;
    for (const auto &pair : colors) {
      const double &current_idx = std::get<0>(pair);
      if (current_idx > index) {
        double nom = (index - processed_index);
        double denom = (current_idx - processed_index);
        double color1_mult = nom / denom;
        double color2_mult = 1.0 - color1_mult;
        color color1 = std::get<1>(colors[counter]);
        color color2 = std::get<1>(colors[counter - 1]);
        return color{(color1.r * color1_mult) + (color2.r * color2_mult),
                     (color1.g * color1_mult) + (color2.g * color2_mult),
                     (color1.b * color1_mult) + (color2.b * color2_mult),
                     (color1.a * color1_mult) + (color2.a * color2_mult)};
      } else {
        processed_index = current_idx;
      }
      counter++;
    }
    const color &c = std::get<1>(colors[counter - 1]);
    return color{c.r, c.g, c.b, c.a};
  }

  template <class Archive>
  void serialize(Archive &ar) {
    ar(colors);
  }
};

inline bool operator==(const gradient &lhs, const gradient &rhs) {
  // TODO: verify if this works, otherwise introduce identifiers
  return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(gradient));
}

}  // namespace data
