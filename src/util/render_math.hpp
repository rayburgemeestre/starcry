/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

template <typename T>
constexpr T squared(T num) {
  return (num * num);
}
template <typename T>
constexpr T squared_dist(T num, T num2) {
  return (num - num2) * (num - num2);
}

template <typename T>
constexpr T distance(T x1, T x2, T y1, T y2) {
  return sqrt(squared_dist<T>(x1, x2) + squared_dist<T>(y1, y2));
}

template <typename T, typename double_type>
constexpr int half_chord_length(T radiusOuterCircle, T relativeY) {
  return static_cast<int>(sqrt(static_cast<double_type>(squared(radiusOuterCircle) - squared(relativeY))) + 0.5);
}
template <typename double_type>
inline int round_to_int(double_type in) {
  return static_cast<int>(0.5 + in);
}
