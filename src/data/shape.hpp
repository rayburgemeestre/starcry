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

#include "data/shape_reflect.hpp"

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
  // NOTE: member order is the cereal wire format. serialize() below walks the members in
  // declaration order and cereal writes unnamed values positionally, so reordering these
  // changes what previously-archived data deserializes into. The goldens in
  // test/reflection_test.cpp will catch it.
  // NOTE: this is not a live constraint. zpp_bits looks for a serialize2 typedef to learn
  // the member count, but nothing serializes data::shape through zpp_bits -- cereal does
  // it, via serialize() below. Verified by adding a 53rd member with members<52> still in
  // place: it compiles and the tests pass. Left declared, and deliberately not bumped,
  // because correcting it to 53 would imply it is checked.
  using serialize2 = zpp::bits::members<52>;

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
  double text_size = 10;
  double radius_size = 0;
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
  int level = 0;
  std::string random_hash;
  bool motion_blur = true;
  int warp_width = 0;
  int warp_height = 0;
  int warp_x = 0;
  int warp_y = 0;

  double dist = 0.;
  int steps = 0.;

  // Serialized by walking the members. Order matters: cereal writes unnamed values
  // positionally, so the declaration order above is the wire format. Reordering members
  // changes what previously-archived data deserializes into.
  template <class Archive>
  void serialize(Archive& ar) {
    data::reflect::for_each_field(*this, [&ar](std::string_view, auto& value) { ar(value); });
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

namespace reflect_detail {

// Generic case: anything nlohmann::json can take directly. Covers the 44 double / int /
// bool / std::string / int64_t members plus std::map<int, size_t> indexes.
template <typename T>
inline void emit(nlohmann::json& j, std::string_view name, const T& value) {
  j[std::string(name)] = value;
}

// The eight members that are not plain. Non-template overloads win over the template
// above by exact match, so adding a new special case is a matter of adding an overload.
inline void emit(nlohmann::json& j, std::string_view name, const shape_type& value) {
  j[std::string(name)] = data::shape_type_to_string(value);
}
inline void emit(nlohmann::json& j, std::string_view name, const texture_3d& value) {
  j[std::string(name)] = data::texture_3d::to_str(value.type());
}
inline void emit(nlohmann::json& j, std::string_view name, const zernike_type& value) {
  j[std::string(name)] = data::zernike_type::to_str(value.type());
}
inline void emit(nlohmann::json& j, std::string_view name, const texture_effect& value) {
  j[std::string(name)] = data::texture_effect::to_str(value.type());
}
inline void emit(nlohmann::json& j, std::string_view name, const blending_type& value) {
  j[std::string(name)] = data::blending_type::to_str(value.type());
}

inline void emit(nlohmann::json& j, std::string_view name, const std::vector<std::pair<double, gradient>>& value) {
  auto& arr = j[std::string(name)] = nlohmann::json::array();
  for (const auto& [index, grad] : value) {
    nlohmann::json gradient_json;
    gradient_json["index"] = index;
    nlohmann::json grad_json;
    grad_json["colors"] = nlohmann::json::array();
    for (const auto& [color_index, color] : grad.colors) {
      nlohmann::json color_json;
      color_json["index"] = color_index;
      color_json["r"] = color.r;
      color_json["g"] = color.g;
      color_json["b"] = color.b;
      color_json["a"] = color.a;
      grad_json["colors"].push_back(color_json);
    }
    gradient_json["gradient"] = grad_json;
    arr.push_back(gradient_json);
  }
}

inline void emit(nlohmann::json& j, std::string_view name, const std::vector<std::pair<double, texture>>& value) {
  auto& arr = j[std::string(name)] = nlohmann::json::array();
  for (const auto& [index, tex] : value) {
    nlohmann::json texture_json;
    texture_json["index"] = index;
    texture_json["type"] = tex.type;
    texture_json["effect"] = tex.effect;
    texture_json["size"] = tex.size;
    texture_json["octaves"] = tex.octaves;
    texture_json["persistence"] = tex.persistence;
    texture_json["percentage"] = tex.percentage;
    texture_json["scale"] = tex.scale;
    texture_json["fromX"] = tex.fromX;
    texture_json["begin"] = tex.begin;
    texture_json["end"] = tex.end;
    texture_json["toX"] = tex.toX;
    texture_json["strength"] = tex.strength;
    texture_json["speed"] = tex.speed;
    texture_json["m"] = tex.m;
    texture_json["n"] = tex.n;
    texture_json["rho"] = tex.rho;
    texture_json["theta"] = tex.theta;
    arr.push_back(texture_json);
  }
}

}  // namespace reflect_detail

// Reflection-based equivalent of shape_to_json above.
//
// The hand-written version lists all 52 members explicitly, which has to be kept in step
// with the struct by hand. This walks them instead, so adding a field to data::shape
// serializes it automatically. Verified byte-identical to shape_to_json in
// test/reflection_test.cpp.
inline bool shape_to_json_reflect(const shape& input, nlohmann::json& j) {
  data::reflect::for_each_field(input, [&j](std::string_view name, const auto& value) {
    reflect_detail::emit(j, name, value);
  });
  return true;
}

inline bool shape_to_json(const shape& input, nlohmann::json& j) {
  return shape_to_json_reflect(input, j);
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
