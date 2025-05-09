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

#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/utility.hpp"  // for std::pair
#include "cereal/types/vector.hpp"
#include "nlohmann/json.hpp"
#include "zpp_bits.h"

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
  // TODO: afaik need C++26 to have more than 52
  // Note-to-self: maybe we can refactor this shape class.
  using serialize2 = zpp::bits::members<52>;

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
  int warp_x = 0;
  int warp_y = 0;

  double dist = 0.;
  int steps = 0.;

  template <class Archive>
  void serialize(Archive& ar) {
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
       warp_x,
       warp_y,
       dist,
       steps);
  }
};

inline bool operator==(const shape& lhs, const shape& rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void*>(&lhs), reinterpret_cast<const void*>(&rhs), sizeof(shape));
}

inline bool shapes_to_binary(const std::vector<shape>& input, std::string& string_out) {
  std::cout << "converting this many shapes: " << input.size() << std::endl;
  std::ostringstream os;
  cereal::JSONOutputArchive archive(os);
  archive(input);
  string_out = os.str();
  std::cout << "into this much binary: " << string_out.size() << std::endl;
  return true;
}

inline bool shape_to_json(const shape& input, nlohmann::json& j) {
  j["time"] = input.time;
  j["x"] = input.x;
  j["y"] = input.y;
  j["z"] = input.z;
  j["x2"] = input.x2;
  j["y2"] = input.y2;
  j["z2"] = input.z2;
  j["velocity"] = input.velocity;
  j["vel_x"] = input.vel_x;
  j["vel_y"] = input.vel_y;
  j["type"] = data::shape_type_to_string(input.type);
  j["r"] = input.r;
  j["g"] = input.g;
  j["b"] = input.b;
  j["radius"] = input.radius;
  j["longest_diameter"] = input.longest_diameter;
  j["shortest_diameter"] = input.shortest_diameter;
  j["rotate"] = input.rotate;
  j["text_size"] = input.text_size;
  j["radius_size"] = input.radius_size;
  j["text"] = input.text;
  j["align"] = input.align;
  j["text_fixed"] = input.text_fixed;
  j["text_font"] = input.text_font;

  // This doesn't work
  // j["gradients_"] = input.gradients_;
  j["gradients_"] = nlohmann::json::array();
  for (const auto& [index, gradient] : input.gradients_) {
    nlohmann::json gradient_json;
    gradient_json["index"] = index;

    nlohmann::json grad_json;
    grad_json["colors"] = nlohmann::json::array();
    for (const auto& [index, color] : gradient.colors) {
      nlohmann::json color_json;
      color_json["index"] = index;
      color_json["r"] = color.r;
      color_json["g"] = color.g;
      color_json["b"] = color.b;
      color_json["a"] = color.a;
      grad_json["colors"].push_back(color_json);
    }
    gradient_json["gradient"] = grad_json;
    j["gradients_"].push_back(gradient_json);
  }

  // This doesn't work
  // j["textures"] = input.textures;
  j["textures"] = nlohmann::json::array();
  for (const auto& [index, texture] : input.textures) {
    nlohmann::json texture_json;
    texture_json["index"] = index;
    texture_json["type"] = texture.type;
    texture_json["effect"] = texture.effect;
    texture_json["size"] = texture.size;
    texture_json["octaves"] = texture.octaves;
    texture_json["persistence"] = texture.persistence;
    texture_json["percentage"] = texture.percentage;
    texture_json["scale"] = texture.scale;
    texture_json["fromX"] = texture.fromX;
    texture_json["begin"] = texture.begin;
    texture_json["end"] = texture.end;
    texture_json["toX"] = texture.toX;
    texture_json["strength"] = texture.strength;
    texture_json["speed"] = texture.speed;
    texture_json["m"] = texture.m;
    texture_json["n"] = texture.n;
    texture_json["rho"] = texture.rho;
    texture_json["theta"] = texture.theta;
    j["textures"].push_back(texture_json);
  }

  j["texture_3d_"] = data::texture_3d::to_str(input.texture_3d_.type());
  j["zernike_type_"] = data::zernike_type::to_str(input.zernike_type_.type());
  j["texture_effect_"] = data::texture_effect::to_str(input.texture_effect_.type());
  j["texture_offset_x"] = input.texture_offset_x;
  j["texture_offset_y"] = input.texture_offset_y;
  j["gradient_id_str"] = input.gradient_id_str;
  j["texture_id_str"] = input.texture_id_str;
  j["blending_"] = data::blending_type::to_str(input.blending_.type());
  j["indexes"] = input.indexes;
  j["seed"] = input.seed;
  j["scale"] = input.scale;
  j["recursive_scale"] = input.recursive_scale;
  j["opacity"] = input.opacity;
  j["hue"] = input.hue;
  j["unique_id"] = input.unique_id;
  j["id"] = input.id;
  j["label"] = input.label;
  j["level"] = input.level;
  j["random_hash"] = input.random_hash;
  j["motion_blur"] = input.motion_blur;
  j["warp_width"] = input.warp_width;
  j["warp_height"] = input.warp_height;
  j["warp_x"] = input.warp_x;
  j["warp_y"] = input.warp_y;
  j["dist"] = input.dist;
  j["steps"] = input.steps;
  return true;
}

inline bool shapes_to_json(const std::vector<shape>& input, std::string& string_out) {
  nlohmann::json j;
  j["shapes"] = nlohmann::json::array();
  size_t previous_shape_idx = 0;
  for (const auto& shape : input) {
    nlohmann::json shape_json;
    const auto _ = shape_to_json(shape, shape_json);
    shape_json["has_children"] = false;
    j["shapes"].emplace_back(shape_json);
    auto& previous_shape = j["shapes"][previous_shape_idx];
    if (shape_json["level"].get<int>() == previous_shape["level"].get<int>() + 1) {
      previous_shape["has_children"] = true;
    }
    previous_shape_idx = j["shapes"].size() - 1;
  }
  string_out = j.dump();
  return true;
}

}  // namespace data
