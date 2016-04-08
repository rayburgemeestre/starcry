#pragma once

#include <algorithm>

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
    };

    inline bool operator==(const shape &lhs, const shape &rhs) {
        return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs),
                                reinterpret_cast<const void *>(&rhs),
                                sizeof(shape));
    }

}
