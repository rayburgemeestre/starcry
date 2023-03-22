#include <catch2/catch_test_macros.hpp>

#include "data/color.hpp"
#include "data/gradient.hpp"
#include "data_staging/shape.hpp"

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

#include "util/step_calculator.hpp"
#include <sstream>

std::string steps_to_matrix_string(int n) {
  step_calculator sc(n);

  std::stringstream ss;
  for (auto j=0; j<=n; j++) {
    ss << j << " =";
    int count = 0;
    for (auto i = 0; i < n; i++) {
      ss << " " << (int)sc.do_step(j, i + 1);
      if (sc.do_step(j, i + 1)) {
        count++;
      }
    }
    ss << ", count = " << count << std::endl;
  }
  return ss.str();
}

TEST_CASE( "Testing step calculator" ) {
  REQUIRE("0 = 0 0 0 0 0 0 0 0 0 0, count = 0\n"
          "1 = 0 0 0 0 0 0 0 0 0 1, count = 1\n"
          "2 = 0 0 0 0 1 0 0 0 0 1, count = 2\n"
          "3 = 0 0 1 0 0 1 0 0 0 1, count = 3\n"
          "4 = 0 1 0 0 1 0 1 0 0 1, count = 4\n"
          "5 = 0 1 0 1 0 1 0 1 0 1, count = 5\n"
          "6 = 1 0 1 0 1 1 0 1 0 1, count = 6\n"
          "7 = 1 1 0 1 1 0 1 1 0 1, count = 7\n"
          "8 = 1 1 1 0 1 1 1 1 0 1, count = 8\n"
          "9 = 1 1 1 1 1 1 1 1 0 1, count = 9\n"
          "10 = 1 1 1 1 1 1 1 1 1 1, count = 10\n" == steps_to_matrix_string(10));

  REQUIRE("0 = 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0, count = 0\n"
          "1 = 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1, count = 1\n"
          "2 = 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1, count = 2\n"
          "3 = 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 1, count = 3\n"
          "4 = 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 1, count = 4\n"
          "5 = 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 1, count = 5\n"
          "6 = 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 1 0 0 0 0 0 1, count = 6\n"
          "7 = 0 0 0 1 0 0 0 0 1 0 0 0 0 1 0 0 0 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1, count = 7\n"
          "8 = 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 0 1, count = 8\n"
          "9 = 0 0 1 0 0 0 1 0 0 0 1 0 0 1 0 0 0 1 0 0 0 1 0 0 1 0 0 0 1 0 0 0 1, count = 9\n"
          "10 = 0 0 1 0 0 1 0 0 1 0 0 0 1 0 0 1 0 0 1 0 0 0 1 0 0 1 0 0 1 0 0 0 1, count = 10\n"
          "11 = 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1, count = 11\n"
          "12 = 0 1 0 0 1 0 0 1 0 0 1 0 1 0 0 1 0 0 1 0 0 1 0 1 0 0 1 0 0 1 0 0 1, count = 12\n"
          "13 = 0 1 0 0 1 0 1 0 0 1 0 1 0 0 1 0 1 0 0 1 0 1 0 0 1 0 1 0 0 1 0 0 1, count = 13\n"
          "14 = 0 1 0 1 0 0 1 0 1 0 1 0 0 1 0 1 0 1 0 0 1 0 1 0 1 0 0 1 0 1 0 0 1, count = 14\n"
          "15 = 0 1 0 1 0 1 0 1 0 0 1 0 1 0 1 0 1 0 1 0 0 1 0 1 0 1 0 1 0 1 0 0 1, count = 15\n"
          "16 = 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 0 1, count = 16\n"
          "17 = 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1, count = 17\n"
          "18 = 1 0 1 0 1 0 1 0 1 0 1 1 0 1 0 1 0 1 0 1 0 1 1 0 1 0 1 0 1 0 1 0 1, count = 18\n"
          "19 = 1 0 1 0 1 1 0 1 0 1 0 1 1 0 1 0 1 0 1 1 0 1 0 1 0 1 1 0 1 0 1 0 1, count = 19\n"
          "20 = 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1, count = 20\n"
          "21 = 1 0 1 1 0 1 1 0 1 0 1 1 0 1 1 0 1 1 0 1 0 1 1 0 1 1 0 1 1 0 1 0 1, count = 21\n"
          "22 = 1 0 1 1 0 1 1 0 1 1 0 1 1 0 1 1 0 1 1 0 1 1 0 1 1 0 1 1 0 1 1 0 1, count = 22\n"
          "23 = 1 1 0 1 1 0 1 1 0 1 1 1 0 1 1 0 1 1 0 1 1 1 0 1 1 0 1 1 0 1 1 0 1, count = 23\n"
          "24 = 1 1 0 1 1 1 0 1 1 0 1 1 1 0 1 1 1 0 1 1 0 1 1 1 0 1 1 1 0 1 1 0 1, count = 24\n"
          "25 = 1 1 1 0 1 1 1 0 1 1 1 0 1 1 1 0 1 1 1 0 1 1 1 0 1 1 1 0 1 1 1 0 1, count = 25\n"
          "26 = 1 1 1 0 1 1 1 1 0 1 1 1 1 0 1 1 1 0 1 1 1 1 0 1 1 1 1 0 1 1 1 0 1, count = 26\n"
          "27 = 1 1 1 1 0 1 1 1 1 0 1 1 1 1 1 0 1 1 1 1 0 1 1 1 1 1 0 1 1 1 1 0 1, count = 27\n"
          "28 = 1 1 1 1 1 0 1 1 1 1 1 1 0 1 1 1 1 1 0 1 1 1 1 1 1 0 1 1 1 1 1 0 1, count = 28\n"
          "29 = 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 0 1, count = 29\n"
          "30 = 1 1 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 1 1 1 0 1, count = 30\n"
          "31 = 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1, count = 31\n"
          "32 = 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1, count = 32\n"
          "33 = 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1, count = 33\n" == steps_to_matrix_string(33));
}

#include "util/frame_stepper.hpp"

TEST_CASE( "Testing frame stepper" ) {
  frame_stepper fs;

  REQUIRE(fs.max_step == 1);

  fs.update(1);
  REQUIRE(fs.max_step == 1);
  fs.update(3);
  REQUIRE(fs.max_step == 3);
  fs.update(2);
  REQUIRE(fs.max_step == 3);

  REQUIRE(fs.current_step == 0);

  int steps = 0;
  while (fs.has_next_step()) {
    steps++;
  }
  REQUIRE(steps == 3);

  REQUIRE(fs.current_step == 3);
  REQUIRE(fs.max_step == 3);

  fs.reset();

  REQUIRE(fs.current_step == 0);
  REQUIRE(fs.max_step == 1);
}

TEST_CASE( "Testing frame stepper rewind" ) {
  frame_stepper fs;

  fs.update(3);

  int steps = 0;
  while (fs.has_next_step()) {
    steps++;
  }
  REQUIRE(steps == 3);

  fs.rewind();
  REQUIRE(fs.current_step == 0);
  REQUIRE(fs.max_step == 3);

  steps = 0;
  while (fs.has_next_step()) {
    steps++;
  }
}

TEST_CASE( "Test size of shape" ) {
  using namespace data_staging;
  shape_t a;
  circle c("circle1", 1000, vector2d{0, 0}, 0, 10.);
  auto largest_size = sizeof(c);
  line l("line1", 1000, vector2d{0, 0}, vector2d{10, 10}, 10.);
  largest_size = std::max(largest_size, sizeof(l));
  text t("text1", 1000, vector2d{0, 0}, "Hello World", 10., "align", false);
  largest_size = std::max(largest_size, sizeof(t));
  script s("script1", 1000, vector2d{0, 0});
  largest_size = std::max(largest_size, sizeof(s));
  REQUIRE(sizeof(a) == largest_size + 8 /* bytes */);
}

// sanity check
TEST_CASE( "Copy of circle shape" ) {
  data_staging::circle c("test", 1, vector2d{19, 11}, 10., 5.);
  REQUIRE(c.location_ref().position_ref().x == 19.);
  auto copy = c;
  c.location_ref().position_ref().x++;
  REQUIRE(copy.location_ref().position_ref().x == 19.);
}