#pragma once

#include <algorithm>

namespace data{

    enum class shape_type {
        circle
    };

    //template <typename T>
    struct shape
    {
        double x;
        double y;
        double z;
        shape_type type;
        double radius;
        double radius_size;
    };

    inline bool operator==(const shape &lhs, const shape &rhs) {
        return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs),
                                reinterpret_cast<const void *>(&rhs),
                                sizeof(shape));
    }

}
