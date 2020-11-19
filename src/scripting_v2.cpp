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

void set_rand_seed(double seed) {
  mt_v2.seed(seed);
}

double rand_fun_v2() {
  return (mt_v2() / (double)mt_v2.max());
}
