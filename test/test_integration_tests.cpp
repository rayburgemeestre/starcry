#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>

#include <fmt/core.h>
#include "starcry.h"
#include "util/v8_wrapper.hpp"

std::shared_ptr<v8_wrapper> context;

class sut {
public:
  starcry_options options;
  sut();
  double test_create_image(const std::string& image_name);
};

sut::sut() {
  static std::once_flag flag;
  options.script_file = "input/snowflakes.js";
  std::call_once(flag, [&]() {
    context = std::make_shared<v8_wrapper>(options.script_file);
  });
  options.frame_of_interest = 10;
  options.num_chunks = 1;
  options.preview = false;
  options.output_file = "unittest_image";
  options.gui = false;
  options.notty = true;
}

double sut::test_create_image(const std::string& image_name) {
  options.output_file = image_name;

  context->set_filename(options.script_file);

  std::remove(image_name.c_str());

  starcry sc(options, context);

  auto vp = sc.get_viewpoint();
  vp.canvas_w = 320;
  vp.canvas_h = 240;
  sc.set_viewpoint(vp);

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
  const auto output_file = fmt::format("{}.png", image_name);
  const auto reference_file = fmt::format("reference/{}.png", image_name);
  if (!std::filesystem::exists(reference_file)) {
    std::cout << "copying " << output_file << " to reference file: " << reference_file << std::endl;
    try {
      std::filesystem::copy_file(output_file, reference_file);
    } catch (std::filesystem::filesystem_error& e) {
      std::cout << "could not copy: " << e.what() << std::endl;
    }
  }
  std::system(fmt::format("/usr/bin/compare -metric RMSE {} {} diff.png 1> diff.txt 2>&1", output_file, reference_file)
                  .c_str());
  std::ifstream in("diff.txt");
  std::string s((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
  std::smatch match;
  std::remove("diff.png");
  std::remove("diff.txt");
  double rmse = 1.;
  if (std::regex_search(s, match, std::regex(R"(\d+ \(([\d.]+)\))"))) {
    rmse = std::stod(match[1]);
  }
  return rmse;
}

TEST_CASE("Test simple render image") {
  sut sc;
  sc.options.script_file = "input/snowflakes.js";
  sc.options.frame_of_interest = 10;
  REQUIRE(sc.test_create_image("unittest_001_snowflakes_frame10") == double(0));
}

TEST_CASE("Test simple render image 2") {
  sut sc;
  sc.options.script_file = "input/perlin3.js";
  sc.options.frame_of_interest = 10;
  REQUIRE(sc.test_create_image("unittest_001_perlin3_frame10") < double(0.05));
}
