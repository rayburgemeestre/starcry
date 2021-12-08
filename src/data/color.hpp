/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
//#include <caf/meta/type_name.hpp>
#include <cstring>

namespace data {

struct color {
  double r = 0;
  double g = 0;
  double b = 0;
  double a = 0;

  template <class Archive>
  void serialize(Archive& ar) {
    ar(r, g, b, a);
  }

  void normalize() {
    r = std::min(r, 1.);
    g = std::min(g, 1.);
    b = std::min(b, 1.);
    a = std::min(a, 1.);
  }

  friend std::ostream& operator<<(std::ostream& stream, const color& b) {
    stream << "#color{" << b.r << ", " << b.g << ", " << b.b << ", " << b.a << "}";
    return stream;
  }
};

inline bool operator==(const color& lhs, const color& rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void*>(&lhs), reinterpret_cast<const void*>(&rhs), sizeof(color));
}

}  // namespace data

static inline data::color blend(const data::color& bg, const data::color& fg) {
  const auto a = fg.a + (bg.a * (1. - fg.a) / 1.);
  data::color clr{
      (fg.r * fg.a + bg.r * bg.a * (1. - fg.a) / 1.) / a,
      (fg.g * fg.a + bg.g * bg.a * (1. - fg.a) / 1.) / a,
      (fg.b * fg.a + bg.b * bg.a * (1. - fg.a) / 1.) / a,
      a,
  };
  return clr;
}

// used for progressively rendering something with motion blur
static inline data::color blend_add(const data::color& bg, const data::color& fg) {
  const auto& a = fg.a;
  const auto A = 1. - fg.a;
  data::color clr{(bg.r * A) + (fg.r * a), (bg.g * A) + (fg.g * a), (bg.b * A) + (fg.b * a), bg.a + fg.a};
  return clr;
}
