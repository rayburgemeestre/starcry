/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

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
  switch (type) {
    case data::blending_type::lighten:
      return "lighten";
    case data::blending_type::darken:
      return "darken";
    case data::blending_type::multiply:
      return "multiply";
    case data::blending_type::average:
      return "average";
    case data::blending_type::add:
      return "add";
    case data::blending_type::subtract:
      return "subtract";
    case data::blending_type::difference:
      return "difference";
    case data::blending_type::negation_:
      return "negation_";
    case data::blending_type::screen:
      return "screen";
    case data::blending_type::exclusion:
      return "exclusion";
    case data::blending_type::overlay:
      return "overlay";
    case data::blending_type::softlight:
      return "softlight";
    case data::blending_type::hardlight:
      return "hardlight";
    case data::blending_type::colordodge:
      return "colordodge";
    case data::blending_type::colorburn:
      return "colorburn";
    case data::blending_type::lineardodge:
      return "lineardodge";
    case data::blending_type::linearburn:
      return "linearburn";
    case data::blending_type::linearlight:
      return "linearlight";
    case data::blending_type::vividlight:
      return "vividlight";
    case data::blending_type::pinlight:
      return "pinlight";
    case data::blending_type::hardmix:
      return "hardmix";
    case data::blending_type::reflect:
      return "reflect";
    case data::blending_type::glow:
      return "glow";
    case data::blending_type::phoenix:
      return "phoenix";
    case data::blending_type::hue:
      return "hue";
    case data::blending_type::saturation:
      return "saturation";
    case data::blending_type::color:
      return "color";
    case data::blending_type::luminosity:
      return "luminosity";
    case data::blending_type::normal:
      return "normal";
    default:
      return "";
  }
}

}  // namespace data