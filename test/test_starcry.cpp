#include <catch2/catch_test_macros.hpp>

#include "starcry.h"

TEST_CASE( "Test simple render image" ) {
  starcry_options options;
  options.script_file = "input/snowflakes.js";
  options.frame_of_interest = 10;
  options.num_chunks = 1;
  options.preview = false;
  options.output_file = "unittest_image.bmp";
  options.notty = true;

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
}