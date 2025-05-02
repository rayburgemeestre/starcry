/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scripting.h"
#include "util/logger.h"
#include "util/random.hpp"

#include <iostream>
// #include <random>

void output_fun(const std::string& s) {
  logger(INFO) << "script: " << s << std::endl;
}

// static util::random_generator rand_;
//
// void set_rand_seed(double seed) {
//   rand_.set_seed(seed);
// }

double rand_fun(util::random_generator& rand) {
  return rand.get();
}

std::vector<double> angled_velocity(double angle) {
  const auto degrees_to_radian = [](double degrees) {
    const auto pi = 3.14159265358979323846;
    return degrees * pi / 180.0;
  };
  const auto radian = degrees_to_radian(angle);
  const auto sine = sin(radian);
  const auto cosine = cos(radian);
  const double x = cosine - sine;
  const double y = sine + cosine;
  // normalize vector of x and y
  // const auto length = sqrt(pow(x, 2) + pow(y, 2));
  // return {x / length, y / length};
  return {x, y};
}

std::vector<double> random_velocity(util::random_generator& rand) {
  double x = rand.get();
  double y = 0;

  const auto degrees_to_radian = [](double degrees) {
    const auto pi = 3.14159265358979323846;
    return degrees * pi / 180.0;
  };

  const auto radian = degrees_to_radian(rand.get() * 360.0);
  const auto sine = sin(radian);
  const auto cosine = cos(radian);
  x = x * cosine - y * sine;
  y = x * sine + y * cosine;

  std::vector<double> vec;
  vec.push_back(x);
  vec.push_back(y);
  return vec;
}

double expf_fun(double v, double factor) {
  auto max = factor;
  auto maxexp = log(max + 1.0) / log(2.0);
  auto linear = v;
  auto expf = ((pow(2.0, (linear)*maxexp))-1.0) / max;
  return expf;
}

double logn_fun(double v, double factor) {
  auto max = factor;
  auto maxexp = log(max + 1.0) / log(2.0);
  auto linear = v;
  auto maxpow = pow(2.0, maxexp);
  auto logn = (maxpow - (pow(2.0, (1.0 - linear) * maxexp))) / max;
  return logn;
}

void my_exit(int code) {
  std::exit(code);
}
