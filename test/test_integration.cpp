/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <iostream>
#include <regex>

#include "system_under_test.hpp"

static const constexpr double fuzziness_allowed = 0;

TEST_CASE("Test simple image") {
  sut sc;
  sc.options.script_file = "input/snowflakes.js";
  sc.options.frame_of_interest = 10;
  REQUIRE(sc.test_create_image("unittest_001_snowflakes_frame10") <= double(0.05 * fuzziness_allowed));
  sc.options.frame_of_interest = 11;
  REQUIRE(sc.test_create_image("unittest_001_snowflakes_frame11") <= double(0.05 * fuzziness_allowed));
  REQUIRE(sc.compare("unittest_001_snowflakes_frame10.png", "unittest_001_snowflakes_frame11.png") > double(0.05));
}

TEST_CASE("Test complex image") {
  sut sc;
  sc.options.script_file = "input/script.js";
  sc.options.frame_of_interest = 1;
  REQUIRE(sc.test_create_image("unittest_005_script_frame1") == double(0));
}

TEST_CASE("Test different perlin noise functions") {
  sut sc;
  sc.options.script_file = "input/perlin.js";
  sc.options.frame_of_interest = 10;
  REQUIRE(sc.test_create_image("unittest_00X_perlin_frame10") <= double(0.08 * fuzziness_allowed));
  sc.options.frame_of_interest = 11;
  REQUIRE(sc.test_create_image("unittest_00X_perlin_frame11") <= double(0.08 * fuzziness_allowed));
  REQUIRE(sc.compare("unittest_00X_perlin_frame10.png", "unittest_00X_perlin_frame11.png") > double(0.05));
}

TEST_CASE("Test image with noise") {
  sut sc;
  sc.options.script_file = "input/perlin3.js";
  sc.options.frame_of_interest = 10;
  REQUIRE(sc.test_create_image("unittest_002_perlin3_frame10") <= double(0.08 * fuzziness_allowed));
}

TEST_CASE("Test dedupe + rotate image") {
  sut sc;
  sc.options.script_file = "input/dupes.js";
  sc.options.frame_of_interest = 1;
  // TODO: no idea why fuzziness is required here
  REQUIRE(sc.test_create_image("unittest_006_dupes_frame1") <= double(0.05));
  sc.options.frame_of_interest = 2;
  REQUIRE(sc.test_create_image("unittest_006_dupes_frame2") <= double(0.05));
  sc.options.frame_of_interest = 50;
  REQUIRE(sc.test_create_image("unittest_006_dupes_frame50") <= double(0.05));
}

TEST_CASE("Test rainbow image") {
  sut sc;
  sc.options.script_file = "input/rainbow.js";
  sc.options.frame_of_interest = 120;
  REQUIRE(sc.test_create_image("unittest_008_rainbow_frame120") <= double(0.05 * fuzziness_allowed));
}

TEST_CASE("Test web image") {
  sut sc;
  sc.options.script_file = "input/web.js";
  sc.options.frame_of_interest = 10;
  REQUIRE(sc.test_create_image("unittest_009_web_frame10") <= double(0.05 * fuzziness_allowed));
  sc.options.frame_of_interest = 11;
  REQUIRE(sc.test_create_image("unittest_009_web_frame11") <= double(0.05 * fuzziness_allowed));
  REQUIRE(sc.compare("unittest_009_web_frame10.png", "unittest_009_web_frame11.png") >=
          double(0.05 * fuzziness_allowed));
}

// videos have no optional fuzziness since ffmpeg encoding is using randomness

TEST_CASE("Test simple test video") {
  sut sc;
  sc.options.script_file = "input/test.js";
  // part of the video (full = 175)
  REQUIRE(sc.test_create_video("unittest_003_test_video.h264", 75, 25) <= double(0.01));
}

/**
 * This test is currently non-deterministic for some reason.
 *
TEST_CASE("Test simple blur video") {
  sut sc;
  sc.options.script_file = "input/blur.js";
  REQUIRE(sc.test_create_video("unittest_004_blur_video.h264", 90, 30) <= double(0.003));
}
*/

TEST_CASE("Test simple orbit video") {
  sut sc;
  sc.options.script_file = "input/orbit.js";
  REQUIRE(sc.test_create_video("unittest_010_orbit_video.h264", 100, 25) <= double(0.003));
}

TEST_CASE("Test blending modes image") {
  sut sc;
  sc.options.script_file = "input/blending_types.js";
  sc.options.frame_of_interest = 1;
  REQUIRE(sc.test_create_image("unittest_011_blending_types_frame1") == double(0));
}

TEST_CASE("Test complex render twice.") {
  sut sc;
  sc.options.script_file = "input/kaleidoscope.js";
  sc.options.frame_of_interest = 1;

  REQUIRE(sc.test_create_image("unittest_012_kaleidoscope_frame1") == double(0));

  // interactive mode crashes rendering the same frame twice for this script
  REQUIRE(sc.test_create_image("unittest_012_kaleidoscope_frame1") == double(0));
}

TEST_CASE("Test scale image.") {
  sut sc;
  sc.options.script_file = "input/scale.js";
  sc.options.frame_of_interest = 1;

  REQUIRE(sc.test_create_image("unittest_013_scale_frame1") == double(0));
}

TEST_CASE("Test rendering selected IDs") {
  sut sc;
  sc.options.script_file = "input/test.js";
  // part of the video (full = 175)
  std::vector<int64_t> selected_ids;
  selected_ids.push_back(20);
  REQUIRE(sc.test_create_image("unittest_014_test_frame1", selected_ids) == double(0));
}
