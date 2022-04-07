#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <regex>

#include "starcry.h"

TEST_CASE("Test simple render image") {
  starcry_options options;
  options.script_file = "input/snowflakes.js";
  options.frame_of_interest = 10;
  options.num_chunks = 1;
  options.preview = false;
  options.output_file = "unittest_image";
  options.notty = true;
  std::remove("unittest_image.png");
  starcry sc(options);
  sc.setup_server();
  sc.add_image_command(nullptr,
                       options.script_file,
                       instruction_type::get_raw_image,
                       options.frame_of_interest,
                       options.num_chunks,
                       true,
                       options.preview,
                       true,
                       options.output_file);
  sc.run_server();
  std::string reference_file = "reference/unittest_image.png";
  std::system(
      "/usr/bin/compare -metric RMSE unittest_image.png reference/unittest_image.png diff.png 1> diff.txt 2>&1");
  std::ifstream in("diff.txt");
  std::string s((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
  std::smatch match;
  double rmse = 1.;
  if (std::regex_search(s, match, std::regex(R"(\d+ \(([\d.]+)\))"))) {
    rmse = std::stod(match[1]);
  }
  REQUIRE(rmse == double(0));
}