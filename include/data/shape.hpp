/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <string>

#include <caf/meta/type_name.hpp>

#include "data/gradient.hpp"

namespace data{

    enum class shape_type {
        text,
        circle,
        line,
    };

    // This is not an enum class because it was easier to use this in V8 (or in other words; I didn't know how to use
    //  enums with V8)
    class blending_type {
    public:
        static const constexpr int normal = 0;
        static const constexpr int lighten = 1;
        static const constexpr int darken = 2;
        static const constexpr int multiply = 3;
        static const constexpr int average = 4;
        static const constexpr int add = 5;
        static const constexpr int subtract = 6;
        static const constexpr int difference = 7;
        static const constexpr int negation_ = 8;
        static const constexpr int screen = 9;
        static const constexpr int exclusion = 10;
        static const constexpr int overlay = 11;
        static const constexpr int softlight = 12;
        static const constexpr int hardlight = 13;
        static const constexpr int colordodge = 14;
        static const constexpr int colorburn = 15;
        static const constexpr int lineardodge = 16;
        static const constexpr int linearburn = 17;
        static const constexpr int linearlight = 18;
        static const constexpr int vividlight = 19;
        static const constexpr int pinlight = 20;
        static const constexpr int hardmix = 21;
        static const constexpr int reflect = 22;
        static const constexpr int glow = 23;
        static const constexpr int phoenix = 24;
        static const constexpr int hue = 25;
        static const constexpr int saturation = 26;
        static const constexpr int color = 27;
        static const constexpr int luminosity = 28;

        blending_type() : type_(blending_type::normal)
        {
        }

        blending_type(int type) : type_(type)
        {
        }

        data::blending_type & operator=(const blending_type &other)
        {
            type_ = other.type_;
            return *this;
        }

        const int & type() { return type_; }

    public:
        int type_;
    };

    template<class Processor>
    void serialize(Processor &proc, data::blending_type &x, const unsigned int) {
        proc & x.type_;
    }

    template <class Inspector>
    typename Inspector::result_type inspect(Inspector& f, data::blending_type& x) {
        return f(caf::meta::type_name("data::blending_type"), x.type_);
    }

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
        double text_size;
        std::string text;
        std::string align;
        gradient gradient_;
        blending_type blending_;
    };

    template<class Processor>
    void serialize(Processor &proc, data::shape &x, const unsigned int) {
        proc & x.x;
        proc & x.y;
        proc & x.z;
        proc & x.x2;
        proc & x.y2;
        proc & x.z2;
        proc & x.type;
        proc & x.r;
        proc & x.g;
        proc & x.b;
        proc & x.radius;
        proc & x.text_size;
        proc & x.radius_size;
        proc & x.text;
        proc & x.align;
        proc & x.gradient_;
        proc & x.blending_;
    }

    inline bool operator==(const shape &lhs, const shape &rhs) {
        return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs),
                                reinterpret_cast<const void *>(&rhs),
                                sizeof(shape));
    }

    template <class Inspector>
    typename Inspector::result_type inspect(Inspector& f, data::shape& x) {
        return f(caf::meta::type_name("data::shape"), x.x,
           x.y,
           x.z,
           x.x2,
           x.y2,
           x.z2,
           x.type,
           x.r,
           x.g,
           x.b,
           x.radius,
           x.text_size,
           x.radius_size,
           x.text,
           x.align,
           x.gradient_,
           x.blending_);
    }
}
