/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <string>
#include <unordered_map>

namespace data {

class zernike_type {
public:
  static const constexpr int version1 = 0;
  static const constexpr int version2 = 1;

  zernike_type() : type_(zernike_type::version1) {}

  zernike_type(int type) : type_(type) {}

  data::zernike_type &operator=(const zernike_type &other) {
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

inline std::string zernike_type::to_str(int type) {
  static const std::unordered_map<int, std::string> texture_type_str{
      {data::zernike_type::version1, "version1"},
      {data::zernike_type::version2, "version2"},
  };

  auto it = texture_type_str.find(type);
  return it == texture_type_str.end() ? "" : it->second;
}

}  // namespace data
