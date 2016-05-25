/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "primitives.h"

color::color(double r, double g, double b, double a) {
    set_r(r);
    set_g(g);
    set_b(b);
    set_a(a);
}

double color::get_r() const { return r_; }
double color::get_g() const { return g_; }
double color::get_b() const { return b_; }
double color::get_a() const { return a_; }
void color::set_r(double r) { r_ = r; }
void color::set_g(double g) { g_ = g; }
void color::set_b(double b) { b_ = b; }
void color::set_a(double a) { a_ = a; }
