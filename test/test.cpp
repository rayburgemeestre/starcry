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

// Motion blur the way I've implemented it cannot work with some blending function AFAIK.
// Will have to add different tests that test the motionblur_buffer.hpp class instead.

//TEST_CASE( "Blending motion blur test" ) {
//  // black background
//  data::color bg{0, 0, 0, 1};
//
//  // red object with 50% transparency
//  data::color obj{1, 0, 0, 0.5};
//
//  // blending them without motion blur
//  REQUIRE( blend(bg, obj) == data::color{0.5, 0, 0, 1} );
//
//  // motion over two frames
//  data::color temp{0, 0, 0, 0};
//  data::color obj_frame_1{1, 0, 0, 0.25};
//  data::color obj_frame_2{1, 0, 0, 0.25};
//
//  // stitching all the frames
//  data::color pix = blend_add(temp, obj_frame_1);
//  pix = blend_add(pix, obj_frame_2);
//
//  // should result in the original object color
//  REQUIRE( pix == data::color{1, 0, 0, 0.5} );
//
//  // and finally produce the same result
//  REQUIRE( blend(bg, pix) == data::color{0.5, 0, 0, 1} );
//}
//
//TEST_CASE( "Blending motion blur test 2" ) {
//  // black background
//  data::color bg{0, 0, 0, 1};
//
//  // motion over two frames
//  data::color temp{0, 0, 0, 0};
//  data::color obj_frame_1{1, 0, 0, 0.25};
//  data::color obj_frame_2{1, 0, 0, 0.25};
//
//  // only one of the frames gets the color
//  data::color pix = blend_add(temp, obj_frame_1);
//
//  // should result in the original object color
//  REQUIRE( pix == data::color{1, 0, 0, 0.25} );
//
//  // and finally produce the same result
//  REQUIRE( blend(bg, pix) == data::color{0.5, 0, 0, 0.5} );
//}

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
