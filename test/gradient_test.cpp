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

    for (int i=0; i<20; i++) {
        double index = i/20.;
        const color &c = grad.get(index);
        std::cout << c.get_r() << " " << c.get_g() << " " << c.get_b() << std::endl;
    }
}

TEST(gradient_test, simple_gradient_2) {
    gradient grad;
    grad.add(0.00, 1, 0, 0, 0);
    grad.add(1.00, 0, 1, 0.5, 0);

    const color &c = grad.get(0.5);
    ASSERT_EQ(0.5, c.get_r());
    ASSERT_EQ(0.5, c.get_g());
    ASSERT_EQ(0.25, c.get_b());

    const color &c2 = grad.get(0.6667);
    ASSERT_EQ("0.333300", std::to_string(c2.get_r()));
    ASSERT_EQ("0.666700", std::to_string(c2.get_g()));
    ASSERT_EQ("0.333350", std::to_string(c2.get_b()));
}
