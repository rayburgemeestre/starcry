/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>

#include "data/gradient.hpp"

namespace data{

    enum class shape_type {
        text,
        circle,
        line,
    };

    //template <typename T>
    struct shape
    {
        double x;
        double y;
        double z;
        double x2;
        double y2;
        double z2;
        shape_type type;
        double r;
        double g;
        double b;
        double radius;
        double radius_size;
        std::string text;
        std::string align;
        gradient gradient_;
    };

    inline bool operator==(const shape &lhs, const shape &rhs) {
        return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs),
                                reinterpret_cast<const void *>(&rhs),
                                sizeof(shape));
    }

}
