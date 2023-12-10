/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <cmath>

class vector2d {
public:
  double x = 0;
  double y = 0;

  vector2d() = default;
  vector2d(double x, double y) : x(x), y(y) {}

  void rotate(double degrees) {
    const double radian = degrees_to_radian(degrees);
    const double sine = std::sin(radian);
    const double cosine = std::cos(radian);
    x = x * cosine - y * sine;
    y = x * sine + y * cosine;
  }
  double degrees_to_radian(double degrees) {
    const auto pi = 3.14159265358979323846;
    return degrees * pi / 180.0;
  }
  double distance_to(const vector2d& other) {
    return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
  }
};

inline vector2d add_vector(vector2d a, vector2d b) {
  return vector2d(a.x + b.x, a.y + b.y);
}

inline vector2d subtract_vector(vector2d a, vector2d b) {
  return vector2d(a.x - b.x, a.y - b.y);
}

inline vector2d divide_vector(vector2d v, double d) {
  return vector2d(v.x / d, v.y / d);
}

inline vector2d multiply_vector(vector2d v, double s) {
  return vector2d(v.x * s, v.y * s);
}

inline vector2d middle_of_vectors(const vector2d& a, const vector2d& b) {
  return vector2d(((a.x - b.x) / 2) + b.x, ((a.y - b.y) / 2) + b.y);
}

inline double dot_product(vector2d a, vector2d b) {
  return a.x * b.x + a.y * b.y;
}

inline double vector_length(vector2d v) {
  return sqrt(dot_product(v, v));
}

inline vector2d unit_vector(vector2d v) {
  const auto length = vector_length(v);
  if (length != 0) return divide_vector(v, length);
  return vector2d(v.x, v.y);
}
