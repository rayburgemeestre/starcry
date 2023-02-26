#pragma once

#include <cmath>
#include <set>
#include <tuple>

class unique_group {
public:
  void add(double x, double y) {
    seen.emplace(x, y);
  }

  void add(double x, double y, double x2, double y2) {
    // for lines add both directions
    seen2.emplace(x, y, x2, y2);
    seen2.emplace(x2, y2, x, y);
  }

  void query(auto destroy_callback, auto... args) {
    auto key = std::make_tuple(args...);
    std::get<0>(key) = std::round(std::get<0>(key) * 4) / 4;
    std::get<1>(key) = std::round(std::get<1>(key) * 4) / 4;
    if (std::get<0>(key) == -0) std::get<0>(key) = 0;
    if (std::get<1>(key) == -0) std::get<1>(key) = 0;
    if constexpr (sizeof...(args) == 2) {
      if (seen.find(key) != seen.end()) {
        destroy_callback();
      } else {
        seen.emplace(key);
      }
    }
    if constexpr (sizeof...(args) == 4) {
      std::get<2>(key) = std::round(std::get<2>(key) * 4) / 4;
      std::get<3>(key) = std::round(std::get<3>(key) * 4) / 4;
      if (std::get<2>(key) == -0) std::get<2>(key) = 0;
      if (std::get<3>(key) == -0) std::get<3>(key) = 0;

      if (seen2.find(key) != seen2.end()) {
        destroy_callback();
      } else {
        seen2.emplace(key);
      }
    }
  }

private:
  std::set<std::tuple<double, double>> seen;
  std::set<std::tuple<double, double, double, double>> seen2;
};
