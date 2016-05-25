/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>

#include "allegro5/color.h"

namespace data {

    struct pixel_data {
        std::vector<::ALLEGRO_COLOR> pixels;
    };

    inline bool operator==(const pixel_data& lhs, const pixel_data& rhs) {
        return std::equal(lhs.pixels.begin(), lhs.pixels.end(), rhs.pixels.begin(), rhs.pixels.end());
    }


}

inline bool operator==(const ALLEGRO_COLOR& lhs, const ALLEGRO_COLOR& rhs) {
    return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs),
                            reinterpret_cast<const void *>(&rhs),
                            sizeof(ALLEGRO_COLOR));
}
