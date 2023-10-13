/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <fmt/core.h>
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
  explicit random_generator() : random_generator(0) {}

  explicit random_generator(double seed) {
    set_seed(seed);
  }

  void set_seed(double seed) {
    mt.seed(seed);
    initialize();
  }

  double get() {
    if (index >= n) {
      index = 0;
    }
    // if (numbers.size() < m) {
    //   throw std::runtime_error(fmt::format("random_generator::get() numbers.size() < m, {} < {}", numbers.size(),
    //   m));
    // }
    // if (numbers.empty()) {
    //   throw std::runtime_error("random_generator::get() numbers.empty() == true");
    // }
    // // add early exit to prevent out of bounds access
    // auto sz = numbers.size();
    // if (index >= sz) {
    //   throw std::runtime_error(fmt::format("random_generator::get() index >= numbers.size(), {} >= {}", index, sz));
    // }
    return numbers[index++];
  }

  void set_index(size_t user_index) {
    index = user_index;
  }

private:
  void reset() {
    index = 0;
  }

  void initialize() {
    std::scoped_lock lock(mut);
    reset();
    numbers.resize(m);
    for (size_t i = 0; i < m; i++) {
      numbers[i] = (mt() / (double)mt.max());
    }
  }
};
}  // namespace util