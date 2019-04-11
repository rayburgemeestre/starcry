/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "primitives.h"

rectangle::rectangle(pos p, double width, double height, gradient grad, int bt)
    : shape(bt), gradient_(grad)
{
    set_x(p.get_x());
    set_y(p.get_y());
    set_z(p.get_z());
    set_width(width);
    set_height(height);
}

double rectangle::get_width() const { return width_; }
void rectangle::set_width(double r) { width_ = r; }
double rectangle::get_height() const { return height_; }
void rectangle::set_height(double r) { height_ = r; }
gradient rectangle::get_gradient() const { return gradient_; }
void rectangle::set_gradient(gradient c) { gradient_ = c; }

bool rectangle::contains(const shape &point) {
    return (point.get_x() >= get_x() && point.get_x() < (get_x() + get_width()) &&
            point.get_y() >= get_y() && point.get_y() < (get_y() + get_height()));
}
