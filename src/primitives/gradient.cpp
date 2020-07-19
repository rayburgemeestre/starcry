/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "primitives.h"

using namespace std;

gradient::gradient() {}
#include <iostream>
const color gradient::get(double index) {
  size_t counter = 0;
  double processed_index = 0;
  for (const auto &pair : colors) {
    const double &current_idx = pair.first;
    if (current_idx > index) {
      double nom = (index - processed_index);
      double denom = (current_idx - processed_index);
      double color1_mult = nom / denom;
      double color2_mult = 1.0 - color1_mult;

      // copies
      color color1 = colors[counter].second;
      color color2 = colors[counter - 1].second;

      //            color1.set_r(color1.get_r() * color1_mult);
      //            color1.set_g(color1.get_g() * color1_mult);
      //            color1.set_b(color1.get_b() * color1_mult);
      //            //color1.set_a(color1.get_a() * color1_mult);
      //
      //            color2.set_r(color2.get_r() * color2_mult);
      //            color2.set_g(color2.get_g() * color2_mult);
      //            color2.set_b(color2.get_b() * color2_mult);
      //            //color2.set_a(color2.get_a() * color2_mult);
      //
      //            color mixed(0, 0, 0, 1. - (1. - color2.get_a()) * (1. - color1.get_a()));
      //
      //            /*var base = [69, 109, 160, 1];
      //              var added = [61, 47, 82, 0.8];
      //
      //            var mix = [];
      //               OK mix[3] = 1 - (1 - added[3]) * (1 - base[3]); // alpha
      //
      //                mix[0] = Math.round((added[0] * added[3] / mix[3]) + (base[0] * base[3] * (1 - added[3]) /
      //                mix[3])); // red mix[1] = Math.round((added[1] * added[3] / mix[3]) + (base[1] * base[3] * (1 -
      //                added[3]) / mix[3])); // green mix[2] = Math.round((added[2] * added[3] / mix[3]) + (base[2] *
      //                base[3] * (1 - added[3]) / mix[3])); // blue
      //                */
      //            mixed.set_r((color2.get_r() * color2.get_a() / mixed.get_a()) + (color1.get_r() * color1.get_a() *
      //            (1. - color2.get_a()) / mixed.get_a())); mixed.set_g((color2.get_g() * color2.get_a() /
      //            mixed.get_a()) + (color1.get_g() * color1.get_a() * (1. - color2.get_a()) / mixed.get_a()));
      //            mixed.set_b((color2.get_b() * color2.get_a() / mixed.get_a()) + (color1.get_b() * color1.get_a() *
      //            (1. - color2.get_a()) / mixed.get_a())); return mixed;
      return color((color1.get_r() * color1_mult) + (color2.get_r() * color2_mult),
                   (color1.get_g() * color1_mult) + (color2.get_g() * color2_mult),
                   (color1.get_b() * color1_mult) + (color2.get_b() * color2_mult),
                   (color1.get_a() * color1_mult) + (color2.get_a() * color2_mult));
    } else {
      processed_index = current_idx;
    }
    counter++;
  }
  color &c = colors[counter - 1].second;
  return color(c.get_r(), c.get_g(), c.get_b(), c.get_a());
}

// temporary test
double gradient::get_r(double index) {
  return get(index).get_r();
}
double gradient::get_g(double index) {
  return get(index).get_g();
}
double gradient::get_b(double index) {
  return get(index).get_b();
}
double gradient::get_a(double index) {
  return get(index).get_a();
}
