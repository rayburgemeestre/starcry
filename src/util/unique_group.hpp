#pragma once

#include <cmath>
#include <set>
#include <tuple>

class unique_group {
public:
  void add(double x, double y) {
    seen.emplace(_round(x), _round(y));
  }

  void add(double x, double y, double x2, double y2) {
    x = _round(x);
    y = _round(y);
    x2 = _round(x2);
    y2 = _round(y2);
    // for lines add both directions
    seen2.emplace(x, y, x2, y2);
    seen2.emplace(x2, y2, x, y);
  }

  void query(auto destroy_callback, auto... args) {
    auto key = std::make_tuple(args...);
    std::get<0>(key) = _round(std::get<0>(key));
    std::get<1>(key) = _round(std::get<1>(key));
    if constexpr (sizeof...(args) == 2) {
      if (seen.find(key) != seen.end()) {
        destroy_callback();
      } else {
        seen.emplace(key);
      }
    }
    if constexpr (sizeof...(args) == 4) {
      std::get<2>(key) = _round(std::get<2>(key));
      std::get<3>(key) = _round(std::get<3>(key));
      if (seen2.find(key) != seen2.end()) {
        destroy_callback();
      } else {
        seen2.emplace(key);
      }
    }
  }

private:
  double _round(double in) {
    // round to nearest .25
    in = std::round(in * 4) / 4;
    if (in == -0) in = 0;
    return in;
  }

  std::set<std::tuple<double, double>> seen;
  std::set<std::tuple<double, double, double, double>> seen2;
};
