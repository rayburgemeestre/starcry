/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <experimental/filesystem>

#include "cereal/archives/binary.hpp"
#include "piper.h"
#include "starcry.h"
#include "starcry_interactive.h"
#include "streamer_output/sfml_window.h"
#include "util/a.hpp"
#include "util/fps_progress.hpp"

starcry::starcry(const std::string &input_script, const std::string &output_file)
    : gen(nullptr), input_script(input_script), output_file(output_file) {
  engine.initialize();
}

void starcry::set_script(const std::string &script) {
  input_script = script;
}
void starcry::set_output(const std::string &output) {
  output_file = output;
}
void starcry::set_custom_max_frames(size_t max_frames) {
  custom_max_frames = max_frames;
}

starcry::render_frame_result starcry::render_frame(size_t frame_of_interest) {
  starcry::render_frame_result res;
  if (!gen) {
    gen = std::make_unique<generator>([&](size_t bitrate, int w, int h, int fps) {},
                                      [&](const data::job &job) {
                                        if (frame_of_interest != job.frame_number) {
                                          return true;  // fast forward
                                        }
                                        auto bmp = bitmap.get(job.width, job.height);

                                        std::ostringstream os;
                                        {
                                          cereal::BinaryOutputArchive archive(os);
                                          archive(job);
                                        }
                                        res.definition_bytes = os.str().length();
                                        render_job(engine, job, bmp);
                                        if (output_file == "output.h264") {
                                          engine.write_image(bmp, "output.bmp");
                                        } else {
                                          engine.write_image(bmp, output_file);
                                        }
                                        return false;  // done
                                      });
  }
  gen->init(input_script);
  auto before = std::chrono::high_resolution_clock::now();
  while (gen->generate_frame())
    ;
  auto after = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> idle = after - before;
  res.time = idle.count() / 1000.0;
  return res;
}

double starcry::render_video(render_video_mode mode) {
  auto stream_mode = frame_streamer::stream_mode::FILE;
  std::optional<int> use_fps;
  if (output_file.substr(output_file.size() - 4, 4) == "m3u8") {
    stream_mode = frame_streamer::stream_mode::HLS;
    use_fps = 1000;
  }
  std::unique_ptr<frame_streamer> framer;
  std::unique_ptr<sfml_window> gui;
  if (mode == render_video_mode::video_only || mode == render_video_mode::video_with_gui) {
    framer = std::make_unique<frame_streamer>(output_file, stream_mode);
  }
  if (mode == render_video_mode::gui_only || mode == render_video_mode::video_with_gui) {
    gui = std::make_unique<sfml_window>();
  }
  if (!gen) {
    gen = std::make_unique<generator>(
        [&](size_t bitrate, int w, int h, int fps) {
          if (framer) framer->initialize(bitrate, w, h, use_fps ? *use_fps : fps);
        },
        [&](const data::job &job) {
          if (mode != render_video_mode::generate_only) {
            auto bmp = bitmap.get(job.width, job.height);
            render_job(engine, job, bmp);
            auto pixels = engine.serialize_bitmap2(bmp, job.width, job.height);
            if (framer) framer->add_frame(pixels);
            if (gui) gui->add_frame(job.width, job.height, pixels);
          }
          return true;
        },
        custom_max_frames);
  }
  gen->init(input_script);
  fps_progress progress;
  while (gen->generate_frame()) {
    progress.inc();
  }
  if (framer) framer->finalize();
  if (gui) gui->finalize();
  return progress.final();
}

void starcry::run_benchmarks() {
  std::cout << "running performance tests" << std::endl;
  set_script("input/perf.js");
  set_output("perf.h264");
  // FPS: 50.0724
  set_custom_max_frames(250);
  auto fps = render_video(starcry::render_video_mode::video_only);
  std::cout << "FPS video rendering: " << fps << std::endl;
  reset();
  // FPS: 1580.37
  set_custom_max_frames(2500);
  fps = render_video(starcry::render_video_mode::render_only);
  std::cout << "FPS rendering only: " << fps << std::endl;
  reset();
  // FPS: 122174
  set_custom_max_frames(250000);
  fps = render_video(starcry::render_video_mode::generate_only);
  std::cout << "FPS javascript only: " << fps << std::endl;
}

void starcry::configure_streaming() {
  if (output_file == "output.h264") {
    output_file.assign("webroot/stream/stream.m3u8");
    set_output(output_file);
    // clean up left-over streaming artifacts
    namespace fs = std::experimental::filesystem;
    const fs::path stream_path{"webroot/stream"};
    for (const auto &entry : fs::directory_iterator(stream_path)) {
      const auto filename = entry.path().filename().string();
      // Note to self in the future: non-experimental filesystem can do:
      //  if (entry.is_regular_file())...
      if (filename.rfind("stream.m3u8", 0) == 0) {
        std::cerr << "cleaning up old file: " << filename << std::endl;
        fs::remove(entry.path());
      }
    }
  }
}

void starcry::configure_interactive() {
  interactive_starcry isc;
}

void starcry::reset() {
  gen = nullptr;
}

void starcry::render_job(rendering_engine_wrapper &engine, const data::job &job, ALLEGRO_BITMAP *bmp) {
  engine.render(bmp,
                job.background_color,
                job.shapes,
                job.offset_x,
                job.offset_y,
                job.canvas_w,
                job.canvas_h,
                job.width,
                job.height,
                job.scale);
}
