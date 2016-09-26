/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "primitives.h"

circle::circle(pos p, double radius, double radiussize, gradient grad, int bt)
    : shape(bt), gradient_(grad)
{
    set_x(p.get_x());
    set_y(p.get_y());
    set_z(p.get_z());
    set_radius(radius);
    set_radiussize(radiussize);
}

double circle::get_radius() const { return radius_; }
void circle::set_radius(double r) { radius_ = r; }
double circle::get_radiussize() const { return radiussize_; }
void circle::set_radiussize(double r) { radiussize_ = r; }
gradient circle::get_gradient() const { return gradient_; }
void circle::set_gradient(gradient c) { gradient_ = c; }
