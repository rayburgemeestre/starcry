/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include "cereal/types/tuple.hpp"
#include "cereal/types/vector.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-volatile"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvolatile"
#endif
#include "vivid/vivid.h"
#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

#include "data/color.hpp"

namespace data {
struct gradient {
  std::vector<std::tuple<double, color>> colors;

  gradient() = default;

  [[nodiscard]] color get(double index) const {
    // TODO: refactor, this COPY & PASTE from gradient.cpp
    size_t counter = 0;
    double processed_index = 0;
    if (colors.empty()) {
      return color{0.1, 0.2, 0.3, 0.};
    }
    for (const auto& pair : colors) {
      const double& current_idx = std::get<0>(pair);
      if (current_idx > index) {
        double nom = (index - processed_index);
        double denom = (current_idx - processed_index);
        double color1_mult = nom / denom;
        double color2_mult = 1.0 - color1_mult;
        color color1 = std::get<1>(colors[counter]);
        color color2 = std::get<1>(colors[counter - 1]);
        vivid::rgb_t col1 = {
            (float)color1.r,
            (float)color1.g,
            (float)color1.b,
        };
        vivid::rgb_t col2 = {
            (float)color2.r,
            (float)color2.g,
            (float)color2.b,
        };
        auto lerped = vivid::lerp(col1, col2, color2_mult);
        return color{lerped.r, lerped.g, lerped.b, (color1.a * color1_mult) + (color2.a * color2_mult)};
      } else {
        processed_index = current_idx;
      }
      counter++;
    }
    const color& c = std::get<1>(colors[counter - 1]);
    return color{c.r, c.g, c.b, c.a};
  }

  template <class Archive>
  void serialize(Archive& ar) {
    ar(colors);
  }
};

inline bool operator==(const gradient& lhs, const gradient& rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void*>(&lhs), reinterpret_cast<const void*>(&rhs), sizeof(gradient));
}
}  // namespace data
