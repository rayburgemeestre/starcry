/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <mutex>
#include <random>

namespace util {
class random_generator {
private:
  std::mt19937 mt;
  size_t index = 0;
  std::vector<double> numbers;
  size_t n = 10000;
  size_t m = n + 1;
  std::mutex mut;

public:
  random_generator(double seed = 0) {
    set_seed(seed);
  }

  void set_seed(double seed) {
    mt.seed(seed);
    initialize();
  }

  double get() {
    if (index >= m) {
      index = 0;
    }
    return numbers[index++];
  }

private:
  void reset() {
    index = 0;
  }

  void initialize() {
    std::scoped_lock lock(mut);
    reset();
    numbers.resize(n);
    for (size_t i = 0; i < n; i++) {
      numbers[i] = (mt() / (double)mt.max());
    }
  }
};
}  // namespace util