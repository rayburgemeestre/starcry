/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>
#include <unordered_map>

namespace data {

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

  static inline std::string to_str(int type);

  template <class Archive>
  void serialize(Archive &ar) {
    ar(type_);
  }

public:
  int type_;
};

inline std::string blending_type::to_str(int type) {
  static const std::unordered_map<int, std::string> blending_type_str{{data::blending_type::lighten, "lighten"},
                                                                      {data::blending_type::darken, "darken"},
                                                                      {data::blending_type::multiply, "multiply"},
                                                                      {data::blending_type::average, "average"},
                                                                      {data::blending_type::add, "add"},
                                                                      {data::blending_type::subtract, "subtract"},
                                                                      {data::blending_type::difference, "difference"},
                                                                      {data::blending_type::negation_, "negation_"},
                                                                      {data::blending_type::screen, "screen"},
                                                                      {data::blending_type::exclusion, "exclusion"},
                                                                      {data::blending_type::overlay, "overlay"},
                                                                      {data::blending_type::softlight, "softlight"},
                                                                      {data::blending_type::hardlight, "hardlight"},
                                                                      {data::blending_type::colordodge, "colordodge"},
                                                                      {data::blending_type::colorburn, "colorburn"},
                                                                      {data::blending_type::lineardodge, "lineardodge"},
                                                                      {data::blending_type::linearburn, "linearburn"},
                                                                      {data::blending_type::linearlight, "linearlight"},
                                                                      {data::blending_type::vividlight, "vividlight"},
                                                                      {data::blending_type::pinlight, "pinlight"},
                                                                      {data::blending_type::hardmix, "hardmix"},
                                                                      {data::blending_type::reflect, "reflect"},
                                                                      {data::blending_type::glow, "glow"},
                                                                      {data::blending_type::phoenix, "phoenix"},
                                                                      {data::blending_type::hue, "hue"},
                                                                      {data::blending_type::saturation, "saturation"},
                                                                      {data::blending_type::color, "color"},
                                                                      {data::blending_type::luminosity, "luminosity"},
                                                                      {data::blending_type::normal, "normal"}};

  auto it = blending_type_str.find(type);
  return it == blending_type_str.end() ? "" : it->second;
}

}  // namespace data