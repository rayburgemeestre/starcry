#pragma once

#include <algorithm>
#include <cstring>

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

}
