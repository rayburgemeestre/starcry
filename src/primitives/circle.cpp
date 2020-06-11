/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "primitives.h"

circle::circle(pos p, double radius, double radiussize, gradient grad, int bt) : shape(bt), gradient_(grad) {
  set_x(p.get_x());
  set_y(p.get_y());
  set_z(p.get_z());
  set_radius(radius);
  set_radiussize(radiussize);
}

double circle::get_radius() const {
  return radius_;
}
void circle::set_radius(double r) {
  radius_ = r;
}
double circle::get_radiussize() const {
  return radiussize_;
}
void circle::set_radiussize(double r) {
  radiussize_ = r;
}
gradient circle::get_gradient() const {
  return gradient_;
}
void circle::set_gradient(gradient c) {
  gradient_ = c;
}

bool circle::contains(const shape &point) {
  const auto d = pow((point.get_x() - get_x()), 2.0) + pow((point.get_y() - get_y()), 2.0);
  // TODO: cache rsquared
  // return d <= this.rSquared;
  return d <= pow(/*TODO FIX THIS BY TAKING A CIRCLE AS PARAM */ (get_radius() * 2 + get_radiussize() * 2), 2.0);
}

bool circle::intersects(const rectangle &range) {
  const auto xDist = abs(range.get_x() - get_x());
  const auto yDist = abs(range.get_y() - get_y());

  // radius of the circle
  const auto r = get_radius() + get_radiussize();

  const auto w = range.get_width();
  const auto h = range.get_height();

  const auto edges = pow((xDist - w), 2) + pow((yDist - h), 2);

  // no intersection
  if (xDist > (r + w) || yDist > (r + h)) return false;

  // intersection within the circle
  if (xDist <= w || yDist <= h) return true;

  // intersection on the edge of the circle
  // return edges <= this.rSquared;
  return edges <= pow(get_radius() + get_radiussize(), 2);  // ^- TODO: cache rSquared
}