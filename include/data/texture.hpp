/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <noise.hpp>
#include "cereal/types/tuple.hpp"
#include "cereal/types/vector.hpp"

#include "data/color.hpp"

#include "perlin.h"

namespace data {

struct texture {
  enum noise_type {
    perlin,
    fractal,
    turbulence,
  };
  noise_type type;
  double size;
  int octaves;
  double persistence;
  double percentage;
  double scale;

  double fromX;
  double begin;
  double end;
  double toX;

  double strength;
  // animation speed
  double speed;

  texture() {}

  double get(double ratio, double x, double y, double z, double context_scale = 1., double seed = 1.) const {
    static SimplexNoise noise;
    auto use_type = SimplexNoise::NoiseTypeEnum::PERLINNOISE;
    if (type == noise_type::fractal) {
      use_type = SimplexNoise::NoiseTypeEnum::FRACTALNOISE;
    } else if (type == noise_type::turbulence) {
      use_type = SimplexNoise::NoiseTypeEnum::TURBULENCE;
    }
    double perlin = noise.simplexNoise(use_type,
                                       size,
                                       octaves,
                                       persistence,
                                       percentage,
                                       scale,
                                       x / context_scale,
                                       y / context_scale,
                                       (z * speed) + seed);
    perlin = std::clamp(perlin, 0., 1.0);
    if (strength == 0) {
      return opacity(ratio);
    } else if (strength < 0 && strength > -1.0) {
      // darken
      double use_strength = -1 * strength;

      double basestrength = 1.0 - use_strength;
      return basestrength + (perlin * use_strength) * opacity(ratio);
      /* Conversion table
       *
       * perlin:        0      0.5   0.75  1.0
       * multipl:       0      1     1.75  2    == perlin + 1
       * strength1:     0      1     1.75  2    = multipl * 1
       * strength0.5:   0      0.5   0.88  1    = multipl * (strength * 1.0)
       *
       * test, perlin = 0.75, strength = 0.75,
       * formule dan: 0.75 / 0.5 = 1.5 (multiply),
       *         dus: 1.5 * (0.75 * 1.0) = 1.125 << multiply met kleur.
       *  128 * 1.125 = 144..
       */
      // double multipl = (perlin / 0.5);
      // return multipl * _strength;
    } else if (strength > 0 && strength <= 1.0) {
      // the 1.0 doesn't make sense to me
      // return 1.0 + (perlin * strength) * opacity(ratio);
      // return (perlin * strength) * opacity(ratio);
      double o = opacity(ratio);
      double t = 1.0 - o;
      return ((perlin * strength) * o) + t;
    }
    // return 0.6 + (perlin * 0.4) * opacity(ratio); // ugly if to strong;
    return opacity(ratio);
  }

  double opacity(double ratio) const {
    // (See @GradientWindow)
    // The variables:
    // 0.0 ...................................... 1.0
    //          0.1, 0.2  --------  0.7, 0.8
    //      {fromX, begin} ------ {end, toX}
    //
    double lenFromX = fabs(fromX - begin);
    double lenToX = fabs(toX - end);
    // ,lenFromX = 0.1 (fabs(0.2 - 0.1))
    // ,lenToX = 0.1 (fabs(0.8 - 0.7))

    // so for each ratio, i.e.
    // 0.05: if(ratio < fromX || ratio > toX) { 0.0 }
    // 0.15: else if(ratio < begin) { (ratio - fromX)@0.05 / lenFromX -> 0.5 }
    // 0.50: else if(ratio < end) { 1.0 }
    // 0.75: else if(/*ratio > end*/) { 1.0 - ((ratio - end)@0.05 / lenToX) -> 0.5 }

    if (ratio < fromX || ratio > toX) {
      return 0.0;
    } else if (ratio < begin) {
      return ((ratio - fromX) / lenFromX);
    } else if (ratio < end) {
      return 1.0;
    } else {
      return 1.0 - ((ratio - end) / lenToX);
    }
  }

  template <class Archive>
  void serialize(Archive &ar) {
    ar(type, size, octaves, persistence, percentage, scale, fromX, begin, end, toX, strength, speed);
  }
};

inline bool operator==(const texture &lhs, const texture &rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(texture));
}

}  // namespace data
