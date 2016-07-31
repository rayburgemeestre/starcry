/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

struct pos
{
    double x_;
    double y_;
    double z_;

    explicit pos(double x, double y, double z) {
        x_ = x;
        y_ = y;
        z_ = z;
    }
    double get_x() const { return x_; }
    double get_y() const { return y_; }
    double get_z() const { return z_; }
    void set_x(double x) { x_ = x; }
    void set_y(double y) { y_ = y; }
    void set_z(double z) { z_ = z; }
};


struct shape
{
    double x_;
    double y_;
    double z_;

    double get_x() const { return x_; }
    double get_y() const { return y_; }
    double get_z() const { return z_; }
    void set_x(double x) { x_ = x; }
    void set_y(double y) { y_ = y; }
    void set_z(double z) { z_ = z; }
};

struct color
{
    double r_;
    double g_;
    double b_;
    double a_;

    explicit color(double r, double g, double b, double a);

    double get_r() const;
    double get_g() const;
    double get_b() const;
    double get_a() const;
    void set_r(double r);
    void set_g(double g);
    void set_b(double b);
    void set_a(double a);
};

#include <vector>

#include "data/gradient.hpp"
#include <v8.h>
class gradient
{
private:
    std::vector<std::pair<double, color>> colors;

public:
    gradient();

    template <typename... Args>
    void add(double index, Args &&... args) {
        colors.emplace_back(std::make_pair(index, color(std::forward<Args>(args)...)));
    }

    void add_color(double index, const color &col) {
        colors.emplace_back(std::make_pair(index, col));
    }

    const color get(double index); 
    v8::Local<v8::Array> get2(double index); 
    v8::Local<v8::Value> get3(double index); 

    // temporary test
    double get_r(double index);
    double get_g(double index);
    double get_b(double index);
    double get_a(double index);

    /*decltype(colors) data() {
        return colors;
    }
    */
    data::gradient to_data_gradient() {
        data::gradient grad;
        for (const auto &pair : colors) {
            grad.colors.push_back(std::make_pair(pair.first, 
                                                 data::color{pair.second.get_r(), pair.second.get_g(), pair.second.get_b(), pair.second.get_a()}));
        }
        return grad;
    }

};

struct circle : shape
{
    double radius_;
    double radiussize_;
    gradient gradient_;

    explicit circle(pos p, double radius, double radiussize, gradient grad);
    double get_radius() const;
    void set_radius(double r);
    double get_radiussize() const;
    void set_radiussize(double r);
    gradient get_gradient() const;
    void set_gradient(gradient c);
};

struct line : shape
{
    double x2_;
    double y2_;
    double z2_;
    double size_;
    gradient gradient_;

    explicit line(pos p, pos p2, double size, gradient grad);

    double get_x2() const;
    double get_y2() const;
    double get_z2() const;
    double get_size() const;
    void set_x2(double x);
    void set_y2(double y);
    void set_z2(double z);
    void set_size(double size);
    gradient get_gradient() const;
    void set_gradient(gradient c);
};

