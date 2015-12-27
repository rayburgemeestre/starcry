#pragma once

namespace data{

enum class shape_type {
    circle
};

template <typename T>
struct shape
{
    T x;
    T y;
    T z;
    shape_type type;
    T radius;
    T radius_size;
};

}
