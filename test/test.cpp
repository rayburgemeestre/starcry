#include <catch2/catch_test_macros.hpp>

#include "data/color.hpp"

#include "data/gradient.hpp"

TEST_CASE( "Blending two colors" ) {
  REQUIRE( blend(data::color{0, 0, 0, 0}, data::color{1, 0, 0, 1}) == data::color{1, 0, 0, 1} );
  REQUIRE( blend(data::color{1, 0, 0, 1}, data::color{0, 1, 0, 1}) == data::color{0, 1, 0, 1} );
}

TEST_CASE( "Blending two colors with opacity" ) {
  // black with half transparent red
  REQUIRE( blend(data::color{0, 0, 0, 1}, data::color{1, 0, 0, 0.5}) == data::color{0.5, 0, 0, 1} );

  // half transparent red on half transparent red
  REQUIRE( blend(data::color{1, 0, 0, 0.5}, data::color{1, 0, 0, 0.5}) == data::color{1, 0, 0, 0.75} );
}

TEST_CASE( "Testing indexing gradient" ) {
  data::gradient grad;
  grad.colors.emplace_back(0., data::color{1, 0, 0, 1});
  grad.colors.emplace_back(1., data::color{1, 0, 0, 0});
  // diminishing red
  REQUIRE( grad.get(0) == data::color{1, 0, 0, 1});
  REQUIRE( grad.get(.5) == data::color{1, 0, 0, 0.5});
}
TEST_CASE( "Testing indexing gradient 2" ) {
  data::gradient grad;
  grad.colors.emplace_back(0., data::color{1, 0, 0, 1});
  grad.colors.emplace_back(1., data::color{1, 0, 0, 1});
  // non-diminishing red
  REQUIRE( grad.get(0) == data::color{1, 0, 0, 1});
  REQUIRE( grad.get(.5) == data::color{1, 0, 0, 1});
}
TEST_CASE( "Testing indexing gradient 3" ) {
  data::gradient grad;
  grad.colors.emplace_back(0., data::color{1, 0, 0, 1});
  grad.colors.emplace_back(1., data::color{0, 0, 0, 0});
  // diminishing red + transitioning to black
  REQUIRE( grad.get(0) == data::color{1, 0, 0, 1});
  REQUIRE( grad.get(.5) == data::color{0.5, 0, 0, 0.5});
}
