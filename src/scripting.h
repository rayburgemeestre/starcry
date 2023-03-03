/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <vector>

namespace util {
class random_generator;
}

void output_fun(const std::string& s);
std::vector<double> angled_velocity(double angle);
std::vector<double> random_velocity(util::random_generator& rand);

double rand_fun(util::random_generator& rand);

double expf_fun(double v, double factor);
double logn_fun(double v, double factor);

void my_exit(int code);
