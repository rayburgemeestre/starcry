/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <optional>
#include "data/shape.hpp"

std::optional<std::pair<double, double>> map_spherical(double x, double y, double r);

std::optional<std::pair<double, double>> map_radial(
    double x_in, double y_in, double center_x, double center_y, double r, int mapping_type);

std::optional<std::tuple<double, double, double>> map_noise(const data::shape& shape, double x, double y, double r);
