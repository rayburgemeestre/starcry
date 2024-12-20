/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "gtest/gtest.h"

#include <vector>

#include "primitives.h"

using namespace std;

TEST(gradient_test, simple_gradient) {
  gradient grad;
  grad.add(0.00, 1, 0, 0, 0);
  grad.add(1.00, 0, 0, 0, 0);

  //    for (int i=0; i<20; i++) {
  //        double index = i/20.;
  //        const color &c = grad.get(index);
  //        std::cout << c.get_r() << " " << c.get_g() << " " << c.get_b() << " " << c.get_a() << std::endl;
  //    }
}

TEST(gradient_test, simple_gradient_2) {
  gradient grad;
  grad.add(0, 1, 0, 0, 1.0);
  grad.add(1, 0, 1, 0.5, 1.0);

  const color &c = grad.get(0.5);
  ASSERT_EQ(0.5, c.get_r());
  ASSERT_EQ(0.5, c.get_g());
  ASSERT_EQ(0.25, c.get_b());
  ASSERT_EQ(1.00, c.get_a());

  const color &c2 = grad.get(0.6667);
  ASSERT_EQ("0.333300", std::to_string(c2.get_r()));
  ASSERT_EQ("0.666700", std::to_string(c2.get_g()));
  ASSERT_EQ("0.333350", std::to_string(c2.get_b()));
}

TEST(gradient_test, test_alpha) {
  gradient grad;
  grad.add(0, color(1, 0, 0, 1));
  grad.add(1, color(1, 0, 0, 0));  // transparent has the same r, g, b,

  // we expect color(1.0, 0, 0, 0.5); (red width 50% transparency)
  const color &c = grad.get(0.5);
  ASSERT_EQ(1.0, c.get_r());
  ASSERT_EQ(0.0, c.get_g());
  ASSERT_EQ(0.0, c.get_b());
  ASSERT_EQ(0.5, c.get_a());
}

TEST(gradient_test, test_alpha_2) {
  gradient grad;
  grad.add(0, color(0.1, 0.2, 0.3, 1));
  grad.add(1, transparency(0.5));  // transparency should inherit r,g,b from previous color in the gradient

  const color &c = grad.get(0.5);
  ASSERT_EQ(0.1, c.get_r());
  ASSERT_EQ(0.2, c.get_g());
  ASSERT_EQ(0.3, c.get_b());
  ASSERT_EQ(0.75, c.get_a());
}
