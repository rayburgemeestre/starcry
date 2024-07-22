/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <cmath>

#include "data/texture_3d.hpp"
#include "noise.hpp"
#include "noise_mappers.h"

std::optional<std::pair<double, double>> map_spherical(double x, double y, double r) {
  const auto imageWidth = r;
  const auto sineLatitude = x / r;
  const auto latitude = std::asin(sineLatitude);
  const auto circleRadius = r * std::cos(latitude);
  const auto sineLongitude = x / circleRadius;
  const auto pi = 3.14159265358979323846;
  if (sineLongitude >= -1.0 && sineLongitude <= 1.0) {
    const auto longitude = std::asin(sineLongitude) + pi / 2.;
    const auto mapped_x = std::floor((longitude * imageWidth / 1.) / pi);
    const auto mapped_y = std::floor(y);
    return std::make_pair(mapped_x, mapped_y);
  }
  return std::nullopt;
}

std::optional<std::pair<double, double>> map_radial(
    double x_in, double y_in, double center_x, double center_y, double r, int mapping_type) {
  const auto dx = x_in - center_x;
  const auto dy = y_in - center_y;
  const auto distanceSquared = dx * dx + dy * dy;
  const auto distance = std::sqrt(distanceSquared);

  if (distance <= r) {
    const auto zSquared = r * r - distanceSquared;
    const auto z = std::sqrt(zSquared);
    const auto zRatio = z / r;

    if (mapping_type == data::texture_3d::radial_displacement) {
      const auto x_out = x_in + dx * zRatio;
      const auto y_out = y_in + dy * zRatio;
      return std::make_pair(x_out, y_out);
    } else if (mapping_type == data::texture_3d::radial_compression) {
      const auto x_out = center_x + dx * (1. - zRatio);
      const auto y_out = center_y + dy * (1. - zRatio);
      return std::make_pair(x_out, y_out);
    } else if (mapping_type == data::texture_3d::radial_distortion) {
      const auto translationFactor = zRatio * zRatio;  // Apply more translation near the edges
      const auto x_out = center_x + dx * (1. - translationFactor);
      const auto y_out = center_y + dy * (1. - translationFactor);
      return std::make_pair(x_out, y_out);
    } else if (mapping_type == data::texture_3d::radial_scaling) {
      // Apply no translation at the center and more translation near the edges
      // this works well for perlin, and fractal, not really for the third one.
      const auto distanceRatio = distance / r;
      const auto pi = 3.14159265358979323846;
      const auto translationFactor = std::sin(distanceRatio * pi / 2);
      const auto x_out = center_x + dx * (1 + translationFactor);
      const auto y_out = center_y + dy * (1 + translationFactor);
      return std::make_pair(x_out, y_out);
    }
  }
  return std::make_pair(dx, dy);
}

// maps points from a 2D plane to 3D Simplex noise coordinates and returns the corresponding noise values along with the
// mapped coordinates
std::optional<std::tuple<double, double, double>> map_noise(const data::shape& shape, double x, double y, double r) {
  const auto imageWidth = r;
  auto nx = x / r;
  auto ny = y / r;
  const auto squaredLength = nx * nx + ny * ny;

  if (squaredLength > 1) return std::nullopt;

  auto nz = std::sqrt(1. - squaredLength);

  static SimplexNoise noise;  // TODO: get rid of static
  const auto noiseValue = noise.noise4d(nx, ny, nz, shape.time);

  // Map the noise value to a range suitable for your texture coordinates
  auto noiseX = std::floor(((noiseValue + 1.) / 2.) * (imageWidth - 1.));
  auto noiseY = std::floor(((noiseValue + 1.) / 2.) * (imageWidth - 1.));

  return std::make_tuple(noiseX, noiseY, noiseValue);
}
