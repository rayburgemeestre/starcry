/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <cstring>

#include <caf/meta/type_name.hpp>

namespace data{

    struct color
    {
        double r;
        double g;
        double b;
        double a;
    };

    inline bool operator==(const color &lhs, const color &rhs) {
        return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs),
                                reinterpret_cast<const void *>(&rhs),
                                sizeof(color));
    }

    template<class Processor>
    void serialize(Processor &proc, data::color &x, const unsigned int) {
        proc & x.r;
        proc & x.g;
        proc & x.b;
        proc & x.a;
    }

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, data::color& x) {
  return f(caf::meta::type_name("data::color"), x.r, x.g, x.b, x.a);
}

}
