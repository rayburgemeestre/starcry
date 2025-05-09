/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace data {

struct settings {
  bool perlin_noise;
  bool motion_blur;
  bool grain_for_opacity;
  double extra_grain;
  bool update_positions;
  bool dithering;
  double brightness;
  double gamma;
  bool scale_ratio;

  settings()
      : perlin_noise(true),
        motion_blur(true),
        grain_for_opacity(true),
        extra_grain(0.1),
        update_positions(true),
        dithering(true),
        brightness(1.0),
        gamma(1.0),
        scale_ratio(true) {}

  template <class Archive>
  void serialize(Archive &ar) {
    ar(perlin_noise,
       motion_blur,
       grain_for_opacity,
       extra_grain,
       update_positions,
       dithering,
       brightness,
       gamma,
       scale_ratio);
  }
};

inline bool operator==(const settings &lhs, const settings &rhs) {
  return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(settings));
}

}  // namespace data
