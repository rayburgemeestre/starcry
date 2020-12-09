/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scripting_v2.h"

#include <iostream>
#include <random>

void output_fun(const std::string& s) {
  std::cout << s << std::endl;
}

std::mt19937 mt_v2;
std::mt19937 mt_v2_b;

void set_rand_seed(double seed) {
  mt_v2.seed(seed);
  mt_v2_b.seed(seed);
}

double rand_fun_v2() {
  return (mt_v2() / (double)mt_v2.max());
}

double rand_fun_v2b() {
  return (mt_v2_b() / (double)mt_v2_b.max());
}

std::vector<double> random_velocity() {
  double x = rand_fun_v2b();
  double y = 0;

  const auto degrees_to_radian = [](double degrees) {
    const auto pi = 3.14159265358979323846;
    return degrees * pi / 180.0;
  };

  const auto radian = degrees_to_radian(rand_fun_v2b() * 360.0);
  const auto sine = sin(radian);
  const auto cosine = cos(radian);
  x = x * cosine - y * sine;
  y = x * sine + y * cosine;

  std::vector<double> vec;
  vec.push_back(x);
  vec.push_back(y);
  return vec;
}
