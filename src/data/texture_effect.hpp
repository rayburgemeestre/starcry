/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>
#include <unordered_map>

namespace data {

class texture_effect {
public:
  static const constexpr int opacity = 0;
  static const constexpr int color = 1;

  texture_effect() : type_(texture_effect::opacity) {}

  texture_effect(int type) : type_(type) {}

  data::texture_effect &operator=(const texture_effect &other) {
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

inline std::string texture_effect::to_str(int type) {
  static const std::unordered_map<int, std::string> texture_effect_str{
      {data::texture_effect::color, "color"},
      {data::texture_effect::opacity, "opacity"},
  };

  auto it = texture_effect_str.find(type);
  return it == texture_effect_str.end() ? "" : it->second;
}

}  // namespace data
