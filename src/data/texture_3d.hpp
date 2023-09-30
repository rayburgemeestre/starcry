/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>
#include <unordered_map>

namespace data {

class texture_3d {
public:
  static const constexpr int raw = 0;
  static const constexpr int radial_displacement = 1;
  static const constexpr int radial_compression = 2;
  static const constexpr int radial_distortion = 3;
  static const constexpr int radial_scaling = 4;
  static const constexpr int spherical = 5;
  static const constexpr int noise_3d_simplex = 6;
  static const constexpr int noise_3d_coords = 7;

  texture_3d() : type_(texture_3d::radial_scaling) {}

  texture_3d(int type) : type_(type) {}

  data::texture_3d &operator=(const texture_3d &other) {
    type_ = other.type_;
    return *this;
  }

  const int &type() const {
    return type_;
  }

  static inline std::string to_str(int type);

  template <class Archive>
  void serialize(Archive &ar) {
    ar(type_);
  }

public:
  int type_;
};

inline std::string texture_3d::to_str(int type) {
  static const std::unordered_map<int, std::string> texture_type_str{
      {data::texture_3d::raw, "raw"},
      {data::texture_3d::radial_displacement, "radial_displacement"},
      {data::texture_3d::radial_compression, "radial_compression"},
      {data::texture_3d::radial_distortion, "radial_distortion"},
      {data::texture_3d::radial_scaling, "radial_scaling"},
      {data::texture_3d::spherical, "spherical"},
      {data::texture_3d::noise_3d_simplex, "noise_3d_simplex"},
      {data::texture_3d::noise_3d_coords, "noise_3d_coords"}};

  auto it = texture_type_str.find(type);
  return it == texture_type_str.end() ? "" : it->second;
}

}  // namespace data
