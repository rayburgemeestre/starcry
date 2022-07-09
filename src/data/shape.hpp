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

#include "data/blending_type.hpp"
#include "data/gradient.hpp"
#include "data/texture.hpp"

namespace data {

enum class shape_type {
  none,
  text,
  circle,
  line,
  script,
};

struct shape {
  double time;
  double x;
  double y;
  double z;
  double x2;
  double y2;
  double z2;
  double vel_x;
  double vel_y;
  shape_type type;
  double r;
  double g;
  double b;
  double radius;
  double radius_size;
  double text_size;
  std::string text;
  std::string align;
  bool text_fixed;
  std::string text_font;
  std::vector<std::pair<double, gradient>> gradients_;
  std::vector<std::pair<double, texture>> textures;
  std::string gradient_id_str;
  blending_type blending_;
  std::map<int, size_t> indexes;
  double seed;
  double scale;
  double opacity;

  int64_t unique_id;

  // annotate
  std::string id;
  std::string label;
  std::string random_hash;
  int level;
  bool motion_blur = true;
  int warp_width = 0;
  int warp_height = 0;

  double dist = 0.;

  template <class Archive>
  void serialize(Archive &ar) {
    ar(time,
       x,
       y,
       z,
       x2,
       y2,
       z2,
       vel_x,
       vel_y,
       type,
       r,
       g,
       b,
       radius,
       text_size,
       radius_size,
       text,
       align,
       text_fixed,
       text_font,
       gradients_,
       textures,
       gradient_id_str,
       blending_,
       indexes,
       seed,
       scale,
       opacity,
       unique_id,
       id,
       label,
       level,
       motion_blur,
       warp_width,
       warp_height,
       dist);
  }
};

inline bool operator==(const shape &lhs, const shape &rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(shape));
}

}  // namespace data
