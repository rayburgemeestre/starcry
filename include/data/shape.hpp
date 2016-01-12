#pragma once

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

// announce requires foo to have the equal operator implemented
inline bool operator==(const shape& lhs, const shape& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y; // TEMP incomplete!
}


}
