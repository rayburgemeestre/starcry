/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "bitmap_wrapper.hpp"
#include "framer.hpp"
#include "generator.h"
#include "rendering_engine_wrapper.h"

class starcry {
private:
  rendering_engine_wrapper engine;
  bitmap_wrapper bitmap;
  std::unique_ptr<generator> gen;
  std::string input_script;
  std::string output_file;
  std::optional<size_t> custom_max_frames;

public:
  struct render_frame_result {
    double time;
    double definition_bytes;
  };

  enum class render_video_mode { generate_only, render_only, video_only, video_with_gui, gui_only };

  starcry(const std::string &input_script, const std::string &output_file);
  void set_script(const std::string &script);
  void set_output(const std::string &output);
  void set_custom_max_frames(size_t max_frames);
  render_frame_result render_frame(size_t frame_of_interest);
  double render_video(render_video_mode mode = render_video_mode::video_only);
  void run_benchmarks();
  void configure_interactive();
  void configure_streaming();
  void reset();

private:
  void render_job(rendering_engine_wrapper &engine, const data::job &job, ALLEGRO_BITMAP *bmp);
};
