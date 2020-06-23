/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "primitives.h"

double get_version();
void set_background_color(color clr);
void add_circle(circle circ);
void add_line(line l);
void add_text(double x, double y, double z, double textsize, std::string text, std::string align);
void output(std::string s);
void write_frame_fun();
void close_fun();
double rand_fun();

// deprecated
void write_frame_fun_impl(bool last_frame);
