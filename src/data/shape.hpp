/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <map>
#include <string>

#include "cereal/types/string.hpp"
#include "cereal/types/utility.hpp"  // for std::pair
#include "cereal/types/vector.hpp"

#include "data/gradient.hpp"
#include "data/texture.hpp"

namespace data {

enum class shape_type {
  none,
  text,
  circle,
  line,
};

// This is not an enum class because it was easier to use this in V8 (or in other words; I didn't know how to use
//  enums with V8)
class blending_type {
public:
  static const constexpr int normal = 0;
  static const constexpr int lighten = 1;
  static const constexpr int darken = 2;
  static const constexpr int multiply = 3;
  static const constexpr int average = 4;
  static const constexpr int add = 5;
  static const constexpr int subtract = 6;
  static const constexpr int difference = 7;
  static const constexpr int negation_ = 8;
  static const constexpr int screen = 9;
  static const constexpr int exclusion = 10;
  static const constexpr int overlay = 11;
  static const constexpr int softlight = 12;
  static const constexpr int hardlight = 13;
  static const constexpr int colordodge = 14;
  static const constexpr int colorburn = 15;
  static const constexpr int lineardodge = 16;
  static const constexpr int linearburn = 17;
  static const constexpr int linearlight = 18;
  static const constexpr int vividlight = 19;
  static const constexpr int pinlight = 20;
  static const constexpr int hardmix = 21;
  static const constexpr int reflect = 22;
  static const constexpr int glow = 23;
  static const constexpr int phoenix = 24;
  static const constexpr int hue = 25;
  static const constexpr int saturation = 26;
  static const constexpr int color = 27;
  static const constexpr int luminosity = 28;

  blending_type() : type_(blending_type::normal) {}

  blending_type(int type) : type_(type) {}

  data::blending_type &operator=(const blending_type &other) {
    type_ = other.type_;
    return *this;
  }

  const int &type() const {
    return type_;
  }

  template <class Archive>
  void serialize(Archive &ar) {
    ar(type_);
  }

public:
  int type_;
};

struct shape {
  double time;
  double x;
  double y;
  double z;
  double x2;
  double y2;
  double z2;
  shape_type type;
  double r;
  double g;
  double b;
  double radius;
  double radius_size;
  double text_size;
  std::string text;
  std::string align;
  std::vector<std::pair<double, gradient>> gradients_;
  std::vector<std::pair<double, texture>> textures;
  blending_type blending_;
  std::map<int, size_t> indexes;
  double seed;
  double scale;
  double opacity;

  int64_t unique_id;

  // annotate
  std::string id;
  std::string label;
  int level;
  bool motion_blur = true;

  template <class Archive>
  void serialize(Archive &ar) {
    ar(time,
       x,
       y,
       z,
       x2,
       y2,
       z2,
       type,
       r,
       g,
       b,
       radius,
       text_size,
       radius_size,
       text,
       align,
       gradients_,
       textures,
       blending_,
       indexes,
       seed,
       scale,
       opacity,
       unique_id,
       id,
       label,
       level,
       motion_blur);
  }
};

inline bool operator==(const shape &lhs, const shape &rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(shape));
}

}  // namespace data
