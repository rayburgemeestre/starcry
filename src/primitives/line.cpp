/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "primitives.h"

line::line(pos p, pos p2, double size, gradient grad) {
    set_x(p.get_x());
    set_y(p.get_y());
    set_z(p.get_z());
    set_x2(p2.get_x());
    set_y2(p2.get_y());
    set_z2(p2.get_z());
    set_size(size);
    set_gradient(grad);
}

double line::get_x2() const { return x2_; }
double line::get_y2() const { return y2_; }
double line::get_z2() const { return z2_; }
double line::get_size() const { return size_; }
gradient line::get_gradient() const { return gradient_; }
void line::set_x2(double x) { x2_ = x; }
void line::set_y2(double y) { y2_ = y; }
void line::set_z2(double z) { z2_ = z; }
void line::set_size(double size) { size_ = size; }
void line::set_gradient(gradient grad) { gradient_ = grad; }

