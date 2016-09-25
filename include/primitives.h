/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "data/gradient.hpp" // TODO: get rid of this dependency
#include <v8.h>
#include <data/shape.hpp>

namespace v8pp {
    class context;
}


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

    static void add_to_context(v8pp::context &ctx);
};

struct shape
{
    double x_;
    double y_;
    double z_;
    int blending_type_;

    double get_x() const { return x_; }
    double get_y() const { return y_; }
    double get_z() const { return z_; }
    int get_blending_type() const { return blending_type_; }
    void set_x(double x) { x_ = x; }
    void set_y(double y) { y_ = y; }
    void set_z(double z) { z_ = z; }
    void set_blending_type(int bt) { blending_type_ = bt; }

    static void add_to_context(v8pp::context &ctx);
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

    static void add_to_context(v8pp::context &ctx);
};

struct transparency {
    double a_;
    transparency(double a);
    double get_a() const;
    void set_a(double a);
};

class gradient
{
private:
    std::vector<std::pair<double, color>> colors;

public:
    gradient();

    void add_color(double index, const color &col) {
        colors.emplace_back(std::make_pair(index, col));
    }

    template <typename... Args>
    void add_impl(double index, std::true_type, transparency &trans, Args &&... args) {
        if (colors.empty()) {
            color color_with_correct_transparency(0, 0, 0, trans.get_a());
            colors.emplace_back(std::make_pair(index, color_with_correct_transparency));
        }
        else {
            color previous_color_with_correct_alpha = colors[colors.size() - 1].second;
            previous_color_with_correct_alpha.set_a(trans.get_a());
            colors.emplace_back(std::make_pair(index, previous_color_with_correct_alpha));
        }
    }
    template <typename... Args>
    void add_impl(double index, std::false_type, Args &&... args) {
        colors.emplace_back(std::make_pair(index, color(std::forward<Args>(args)...)));
    }

    template <typename T, typename... Args>
    void add(double index, T T1, Args &&... args) {
        add_impl(index, std::is_same<transparency, T>(), T1, std::forward<Args>(args)...);
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

    static void add_to_context(v8pp::context &ctx);
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

    static void add_to_context(v8pp::context &ctx);
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

    static void add_to_context(v8pp::context &ctx);
};

