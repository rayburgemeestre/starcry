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
  double test_create_video(const std::string& video_name, int frames, int fps);
  double compare(const std::string& file1, const std::string& file2);
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
  options.output = true;
  options.output_file = "unittest_image";
  options.gui = false;
  options.notty = true;
}

double sut::test_create_image(const std::string& image_name) {
  std::filesystem::create_directories("test/integration/last-run");
  std::filesystem::create_directories("test/integration/reference");
  options.output_file = fmt::format("test/integration/last-run/{}", image_name);
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
  const auto output_file = fmt::format("test/integration/last-run/{}.png", image_name);
  const auto reference_file = fmt::format("test/integration/reference/{}.png", image_name);
  if (!std::filesystem::exists(reference_file)) {
    std::cout << "copying " << output_file << " to reference file: " << reference_file << std::endl;
    try {
      std::filesystem::copy_file(output_file, reference_file);
    } catch (std::filesystem::filesystem_error& e) {
      std::cout << "could not copy: " << e.what() << std::endl;
    }
  }
  double rmse = compare(output_file, reference_file);
  // std::remove(image_name.c_str());
  return rmse;
}

double sut::test_create_video(const std::string& video_name, int frames, int fps) {
  std::filesystem::create_directories("test/integration/last-run");
  std::filesystem::create_directories("test/integration/reference");
  options.output_file = fmt::format("test/integration/last-run/{}", video_name);
  context->set_filename(options.script_file);
  std::remove(video_name.c_str());
  starcry sc(options, context);
  auto vp = sc.get_viewpoint();
  vp.canvas_w = 320;
  vp.canvas_h = 240;
  sc.set_viewpoint(vp);
  sc.setup_server();
  sc.add_video_command(nullptr,
                       options.script_file,
                       options.output_file,
                       options.num_chunks,
                       false,  // is_raw,
                       options.preview,
                       options.frame_offset);
  sc.run_server();

  // convert video to seekable video
  const auto video_stem = fs::path(video_name).stem().string();
  const auto seekable_video = fmt::format("test/integration/last-run/{}.mp4", video_stem);
  const auto grid_image = fmt::format("test/integration/last-run/{}.png", video_stem);
  const auto grid_image_ref = fmt::format("test/integration/reference/{}.png", video_stem);
  const auto stdout_1 = fmt::format("test/integration/reference/{}.stdout", video_stem);
  const auto stderr_1 = fmt::format("test/integration/reference/{}.stderr", video_stem);
  const auto stdout_2 = fmt::format("test/integration/reference/{}.stdout2", video_stem);
  const auto stderr_2 = fmt::format("test/integration/reference/{}.stderr2", video_stem);
  std::remove(seekable_video.c_str());
  std::remove(grid_image.c_str());
  std::system(
      fmt::format(
          "/usr/bin/ffmpeg -y -i \"{}\" -c:v libx264 -profile:v baseline -level 3.0 -pix_fmt yuv420p \"{}\" 1>{} 2>{}",
          options.output_file,
          seekable_video,
          stdout_1,
          stderr_1)
          .c_str());
  const auto mod = frames / fps;  // 5x5 grid = 25 frames
  std::system(fmt::format("/usr/bin/ffmpeg -i {} -vf 'select=not(mod(n\\,{})),scale=320:240,tile=5x5' {} 1>{} 2>{}",
                          seekable_video,
                          mod,
                          grid_image,
                          stdout_2,
                          stderr_2)
                  .c_str());

  if (!std::filesystem::exists(grid_image_ref)) {
    std::cout << "copying " << grid_image << " to reference file: " << grid_image_ref << std::endl;
    try {
      std::filesystem::copy_file(grid_image, grid_image_ref);
    } catch (std::filesystem::filesystem_error& e) {
      std::cout << "could not copy: " << e.what() << std::endl;
    }
  }
  double rmse = compare(grid_image, grid_image_ref);
  // std::remove(video_name.c_str());
  // std::remove(seekable_video.c_str());
  // std::remove(grid_image.c_str());
  return rmse;
}

double sut::compare(const std::string& file1, const std::string& file2) {
  std::system(
      fmt::format("/usr/bin/compare -metric RMSE {} {} /tmp/diff.png 1> /tmp/diff.txt 2>&1", file1, file2).c_str());
  std::ifstream in("/tmp/diff.txt");
  std::string s((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
  std::smatch match;
  std::remove("/tmp/diff.png");
  std::remove("/tmp/diff.txt");
  double rmse = 1.;
  if (std::regex_search(s, match, std::regex(R"(\d+ \(([\d.]+)\))"))) {
    rmse = std::stod(match[1]);
  }
  return rmse;
}

TEST_CASE("Test simple image") {
  sut sc;
  sc.options.script_file = "input/snowflakes.js";
  sc.options.frame_of_interest = 10;
  REQUIRE(sc.test_create_image("unittest_001_snowflakes_frame10") == double(0));
  sc.options.frame_of_interest = 11;
  REQUIRE(sc.test_create_image("unittest_001_snowflakes_frame11") == double(0));
  REQUIRE(sc.compare("unittest_001_snowflakes_frame10.png", "unittest_001_snowflakes_frame11.png") != double(0));
}

TEST_CASE("Test image with noise") {
  sut sc;
  sc.options.script_file = "input/perlin3.js";
  sc.options.frame_of_interest = 10;
  REQUIRE(sc.test_create_image("unittest_002_perlin3_frame10") < double(0.05));
}

TEST_CASE("Test simple test video") {
  sut sc;
  sc.options.script_file = "input/test.js";
  REQUIRE(sc.test_create_video("unittest_003_test_video.h264", 175, 25) < double(0.003));
}

TEST_CASE("Test simple blur video") {
  sut sc;
  sc.options.script_file = "input/blur.js";
  REQUIRE(sc.test_create_video("unittest_004_blur_video.h264", 90, 30) < double(0.003));
}