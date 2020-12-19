/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <fmt/core.h>

#include "framer.hpp"
#include "generator.h"
#include "starcry.h"
#include "starcry/command_get_video.h"
#include "util/progress_visualizer.h"

command_get_video::command_get_video(starcry &sc) : command_handler(sc) {}

void command_get_video::to_job(std::shared_ptr<instruction> &cmd_def) {
  double use_fps = sc.gen->fps();
  if (!sc.framer &&
      (sc.mode == starcry::render_video_mode::video_only || sc.mode == starcry::render_video_mode::video_with_gui) &&
      cmd_def->output_file != "/dev/null") {
    auto stream_mode = frame_streamer::stream_mode::FILE;
    auto output_file = cmd_def->output_file;
    if (output_file.size() >= 4 && output_file.substr(output_file.size() - 4, 4) == "m3u8") {
      use_fps = 1000;
      stream_mode = frame_streamer::stream_mode::HLS;
    }
    if (cmd_def->output_file.empty()) {
      cmd_def->output_file =
          fmt::format("output_seed_{}_{}x{}.h264", sc.gen->get_seed(), (int)sc.gen->width(), (int)sc.gen->height());
    }
    sc.framer = std::make_unique<frame_streamer>(cmd_def->output_file, stream_mode);
  }
  sc.visualizer->initialize();
  sc.visualizer->set_max_frames(sc.gen->get_max_frames());

  // old generator:
  // bitrate = context->run<double>("typeof bitrate != 'undefined' ? bitrate : (500 * 1024 * 8)");
  size_t bitrate = (500 * 1024 * 8);
  if (sc.framer) sc.framer->initialize(bitrate, sc.gen->width(), sc.gen->height(), use_fps);

  while (true) {
    auto ret = sc.gen->generate_frame();
    auto job_copy = std::make_shared<data::job>(*sc.gen->get_job());
    sc.jobs->push(std::make_shared<job_message>(cmd_def->client, cmd_def->type, job_copy));
    sc.jobs->sleep_until_not_full();
    if (!ret) break;
  }
  std::cout << std::endl;
}

std::shared_ptr<render_msg> command_get_video::to_render_msg(std::shared_ptr<job_message> &job_msg, image &bmp) {
  if (job_msg->client != nullptr) {
    return nullptr;
  }
  auto &job = *job_msg->job;
  auto transfer_pixels = sc.pixels_vec_to_pixel_data(bmp.pixels());
  return std::make_shared<render_msg>(
      job_msg->client, job_msg->type, job.job_number, job.width, job.height, transfer_pixels);
}
