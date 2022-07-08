/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <vector>

void output_fun(const std::string& s);
std::vector<double> random_velocity();

void set_rand_seed(double seed);
double rand_fun();

double expf_fun(double v, double factor);
double logn_fun(double v, double factor);
