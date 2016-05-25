#pragma once

#include <algorithm>

#include "data/color.hpp"

namespace data{

    struct gradient
    {
        std::vector<std::pair<double, color>> colors;

        gradient() {}

        color get(double index) {
            // TODO: refactor, this COPY & PASTE from gradient.cpp
            size_t counter = 0;
            double processed_index = 0;
            for (const auto &pair : colors) {
                const double &current_idx = pair.first;
                if (current_idx > index) {
                    double nom          = (index - processed_index);
                    double denom        = (current_idx - processed_index);
                    double color1_mult  = nom/denom;
                    double color2_mult  = 1.0 - color1_mult;
                    const color &color1 = colors[counter    ].second;
                    const color &color2 = colors[counter - 1].second;
                    return color{ (color1.r * color1_mult) + (color2.r * color2_mult),
                                  (color1.g * color1_mult) + (color2.g * color2_mult),
                                  (color1.b * color1_mult) + (color2.b * color2_mult),
                                  (color1.a * color1_mult) + (color2.a * color2_mult) };
                } else {
                    processed_index = current_idx;
                }
                counter++;
            }
            color &c = colors[counter - 1].second;
            return color{ c.r, c.g, c.b, c.a };
        }
    };

    inline bool operator==(const gradient &lhs, const gradient &rhs) {
        // TODO: verify if this works, otherwise introduce identifiers
        return 0 == std::memcmp(reinterpret_cast<const void *>(&lhs),
                                reinterpret_cast<const void *>(&rhs),
                                sizeof(gradient));
    }

}
