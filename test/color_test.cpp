/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "gtest/gtest.h"

#include "color_blender.hpp"
#include "primitives.h"

using namespace std;

TEST(color_test, simple_color) {
  color c(0.1, 0.2, 0.3, 0.4);
  ASSERT_EQ(0.1, c.get_r());
  ASSERT_EQ(0.2, c.get_g());
  ASSERT_EQ(0.3, c.get_b());
  ASSERT_EQ(0.4, c.get_a());
}

TEST(color_test, test_blend_lighten) {
  color color1(1.0, 0.0, 0.0, 1.0);
  color color2(0.0, 0.0, 1.0, 1.0);

  color color3 = blender<lighten>(color1, color2);
  ASSERT_EQ(1.0, color3.get_r());
  ASSERT_EQ(0.0, color3.get_g());
  ASSERT_EQ(1.0, color3.get_b());
  ASSERT_EQ(1.0, color3.get_a());
}

TEST(color_test, test_blend_darken) {
  color color1(1.0, 0.0, 0.0, 1.0);
  color color2(0.0, 0.0, 1.0, 1.0);

  color color3 = blender<darken>(color1, color2);
  ASSERT_EQ(0.0, color3.get_r());
  ASSERT_EQ(0.0, color3.get_g());
  ASSERT_EQ(0.0, color3.get_b());
  ASSERT_EQ(1.0, color3.get_a());
}
