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

    template <class Processor>
    void serialize(Processor& proc, struct data::pixel_data & x, const unsigned int) {
        proc & x.pixels;
    }

    struct pixel_data2 {
        std::vector<uint32_t> pixels;
    };

    template <class Processor>
    void serialize(Processor& proc, struct data::pixel_data2 & x, const unsigned int) {
        proc & x.pixels;
    }

  template <class Inspector>
  typename Inspector::result_type inspect(Inspector& f, data::pixel_data& x) {
    return f(meta::type_name("data::pixel_data"), x.pixels);
  }
template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, data::pixel_data2& x) {
  return f(meta::type_name("data::pixel_data2"), x.pixels);
}

}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, ALLEGRO_COLOR& x) {
  return f(meta::type_name("ALLEGRO_COLOR"), x.a, x.b, x.g, x.r);
}

template <class Processor>
void serialize(Processor& proc, ::ALLEGRO_COLOR & x, const unsigned int) {
    proc & x.r;
    proc & x.g;
    proc & x.b;
    proc & x.a;
}


inline bool operator==(const ALLEGRO_COLOR& lhs, const ALLEGRO_COLOR& rhs) {
    return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs),
                            reinterpret_cast<const void *>(&rhs),
                            sizeof(ALLEGRO_COLOR));
}
