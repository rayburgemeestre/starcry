/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cmath>

// needed since we don't have std::clamp in <algorithm> with em++ currently
template <class T>
inline constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
  //  assert(!(hi < lo));
  return (v < lo) ? lo : (hi < v) ? hi : v;
}

static constexpr const auto pi = 3.14159265358979323846;

inline double squared(const double& num) {
  return num * num;
}

inline double squared_dist(const double& num, const double& num2) {
  return (num - num2) * (num - num2);
}

inline double get_distance(double x, double y, double x2, double y2) {
  return sqrt(squared_dist(x, x2) + squared_dist(y, y2));
}

inline double get_angle(const double& x1, const double& y1, const double& x2, const double& y2) {
  double dx = x1 - x2;
  double dy = y1 - y2;

  if (dx == 0 && dy == 0) return 0;

  if (dx == 0) {
    if (dy < 0)
      return 270;
    else
      return 90;
    // old hack, dx = 1;
  }

  double slope = dy / dx;
  double angle = atan(slope);
  // double angle = atan2(dy, dx); // yields erroneous results.
  if (dx < 0) angle += pi;

  angle = 180.0 * angle / pi;

  while (angle < 0.0) angle += 360.0;

  return angle;
}

inline double triangular_wave(double x, double amplitude = 1., double period = 25.) {
  period *= 2.;
  x += period / 4.;  // start at the top of the wave function
  double y = ((2. * amplitude) / pi) * asin(sin(x * ((2. * pi) / period)));
  return y;
}
