/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <unordered_map>

#include "cereal/types/string.hpp"
#include "cereal/types/utility.hpp"  // for std::pair
#include "cereal/types/vector.hpp"

#include "data/blending_type.hpp"
#include "data/gradient.hpp"
#include "data/texture.hpp"
#include "data/texture_3d.hpp"
#include "data/texture_effect.hpp"
#include "data/zernike_type.hpp"

namespace data {

enum class shape_type {
  none,
  text,
  circle,
  ellipse,
  line,
  script,
};

inline std::string shape_type_to_string(shape_type type) {
  static const std::unordered_map<shape_type, std::string> shape_type_str{{shape_type::none, "none"},
                                                                          {shape_type::text, "text"},
                                                                          {shape_type::circle, "circle"},
                                                                          {shape_type::ellipse, "ellipse"},
                                                                          {shape_type::line, "line"},
                                                                          {shape_type::script, "script"}};

  auto it = shape_type_str.find(type);
  return it == shape_type_str.end() ? "none" : it->second;
}

struct shape {
  // @add_field@
  double time = 0;
  double x = 0;
  double y = 0;
  double z = 0;
  double x2 = 0;
  double y2 = 0;
  double z2 = 0;
  double velocity = 0;
  double vel_x = 0;
  double vel_y = 0;
  shape_type type = shape_type::none;
  double r = 0;
  double g = 0;
  double b = 0;
  double radius = 0;
  double longest_diameter = 0;
  double shortest_diameter = 0;
  double rotate = 0;  // ellipse specific rotation
  double radius_size = 0;
  double text_size = 10;
  std::string text;
  std::string align;
  bool text_fixed = false;
  std::string text_font;
  std::vector<std::pair<double, gradient>> gradients_;
  std::vector<std::pair<double, texture>> textures;
  texture_3d texture_3d_ = texture_3d::raw;
  zernike_type zernike_type_ = zernike_type::version1;
  texture_effect texture_effect_ = texture_effect::opacity;
  double texture_offset_x = 0;
  double texture_offset_y = 0;
  std::string gradient_id_str;
  std::string texture_id_str;
  blending_type blending_ = blending_type::normal;
  std::map<int, size_t> indexes;
  double seed = 0;
  double scale = 1.;
  double recursive_scale = 1.;
  double opacity = 1.;
  double hue = 0.;

  int64_t unique_id = 0;

  // annotate
  std::string id;
  std::string label;
  std::string random_hash;
  int level = 0;
  bool motion_blur = true;
  int warp_width = 0;
  int warp_height = 0;

  double dist = 0.;
  int steps = 0.;

  template <class Archive>
  void serialize(Archive &ar) {
    // @add_field@
    ar(time,
       x,
       y,
       z,
       x2,
       y2,
       z2,
       velocity,
       vel_x,
       vel_y,
       type,
       r,
       g,
       b,
       radius,
       longest_diameter,
       shortest_diameter,
       rotate,
       text_size,
       radius_size,
       text,
       align,
       text_fixed,
       text_font,
       gradients_,
       textures,
       texture_3d_,
       zernike_type_,
       texture_effect_,
       texture_offset_x,
       texture_offset_y,
       gradient_id_str,
       texture_id_str,
       blending_,
       indexes,
       seed,
       scale,
       recursive_scale,
       opacity,
       hue,
       unique_id,
       id,
       label,
       level,
       random_hash,
       motion_blur,
       warp_width,
       warp_height,
       dist,
       steps);
  }
};

inline bool operator==(const shape &lhs, const shape &rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(shape));
}

}  // namespace data
