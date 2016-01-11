#pragma once

#include <vector>

template <typename T>
class rectangle {
private:
    T x_;
    T y_;
    T x2_;
    T y2_;
public:
    rectangle(T x, T y, T x2, T y2) : x_(x), y_(y), x2_(x2), y2_(y2) {}
    inline const T & x() const { return x_; }
    inline const T & y() const { return y_; }
    inline const T & x2() const { return x2_; }
    inline const T & y2() const { return y2_; }
    inline const T width() const { return x2() - x(); };
    inline const T height() const { return y2() - y(); };
};

template <typename T = uint32_t>
class ImageSplitter
{
public:
    enum class Mode {
        SplitAuto = 1,
        SplitHorizontal = 2,
        SplitVertical = 3
    };

    ImageSplitter(T width, T height) : width(width), height(height)
    {}

    inline const std::vector<rectangle<T>> split(uint32_t pairs, Mode mode = Mode::SplitAuto) {
        std::vector<rectangle<T>> ret;
        if (mode == Mode::SplitAuto)
            mode = width > height ? Mode::SplitVertical : Mode::SplitHorizontal;
        uint32_t length = (mode == Mode::SplitVertical ? width : height) / pairs;
        uint32_t current_length = 0;
        for (size_t i=0; i<pairs - 1; i++) {
            if (mode == Mode::SplitVertical)
                ret.push_back(rectangle<T>{current_length, 0, current_length + length, height});
            else
                ret.push_back(rectangle<T>{0, current_length, width, current_length + length});
            current_length += length;
        }
        if (mode == Mode::SplitVertical)
            ret.push_back(rectangle<T>{current_length, 0, width, height});
        else
            ret.push_back(rectangle<T>{0, current_length, width, height});
        return ret;
    }
private:
    const T width;
    const T height;
};

template <typename T>
std::ostream& operator<<(std::ostream& ost, const rectangle<T>& ls)
{
    ost << "(" << ls.x() << "x" << ls.y()  << ", " << ls.x2() << "x" << ls.y2() << ")";
    return ost;
}