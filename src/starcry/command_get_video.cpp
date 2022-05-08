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
#include "util/image_splitter.hpp"
#include "util/image_utils.h"
#include "util/logger.h"

command_get_video::command_get_video(starcry &sc) : command_handler(sc) {}

void command_get_video::to_job(std::shared_ptr<instruction> &cmd_def) {
  double use_fps = sc.gen->fps();
  if (!sc.framer && sc.options().output && cmd_def->output_file != "/dev/null") {
    auto stream_mode = frame_streamer::stream_mode::FILE;
    auto output_file = cmd_def->output_file;
    if (output_file.size() >= 4 && output_file.substr(output_file.size() - 4, 4) == "m3u8") {
      use_fps = 1000;
      stream_mode = frame_streamer::stream_mode::HLS;
    }
    if (cmd_def->output_file.empty()) {
      auto scriptname = fs::path(sc.script_).stem().string();
      cmd_def->output_file = fmt::format(
          "output_seed_{}_{}x{}-{}.h264", sc.gen->get_seed(), (int)sc.gen->width(), (int)sc.gen->height(), scriptname);
    }
    sc.framer = std::make_unique<frame_streamer>(cmd_def->output_file, stream_mode);
    sc.framer->set_log_callback([&](int level, const std::string &line) {
      sc.metrics_->log_callback(level, line);
    });
  }

  // old generator:
  // bitrate = context->run<double>("typeof bitrate != 'undefined' ? bitrate : (500 * 1024 * 8)");
  size_t bitrate = (500 * 1024 * 8);
  if (sc.framer) sc.framer->initialize(bitrate, sc.gen->width(), sc.gen->height(), use_fps);

  while (true) {
    auto ret = sc.gen->generate_frame();
    auto job_copy = std::make_shared<data::job>(*sc.gen->get_job());
    if (job_copy->frame_number < cmd_def->offset_frames) {
      sc.metrics_->skip_job(job_copy->job_number);
      continue;
    }
    // TODO: duplicated code, move to base class
    util::ImageSplitter<uint32_t> is{job_copy->canvas_w, job_copy->canvas_h};
    if (cmd_def->num_chunks == 1) {
      sc.jobs->push(std::make_shared<job_message>(cmd_def->client, cmd_def->type, job_copy, cmd_def->raw));
    } else {
      sc.metrics_->resize_job(job_copy->job_number, cmd_def->num_chunks);
      const auto rectangles = is.split(cmd_def->num_chunks, util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
      for (size_t i = 0, counter = 1; i < rectangles.size(); i++) {
        job_copy->width = rectangles[i].width();
        job_copy->height = rectangles[i].height();
        job_copy->offset_x = rectangles[i].x();
        job_copy->offset_y = rectangles[i].y();
        job_copy->chunk = counter;
        job_copy->num_chunks = cmd_def->num_chunks;
        counter++;
        sc.jobs->push(std::make_shared<job_message>(
            cmd_def->client, cmd_def->type, std::make_shared<data::job>(*job_copy), cmd_def->raw));
      }
    }
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
  auto transfer_pixels = pixels_vec_to_pixel_data(bmp.pixels(), sc.gen->settings().dithering);
  auto raw = job_msg->raw;
  if (raw) {
    return std::make_shared<render_msg>(job_msg->client,
                                        job_msg->type,
                                        job.job_number,
                                        job.frame_number,
                                        job.chunk,
                                        job.num_chunks,
                                        job.offset_x,
                                        job.offset_y,
                                        job.last_frame,
                                        false,
                                        job.width,
                                        job.height,
                                        bmp.pixels(),
                                        "");
  } else {
    return std::make_shared<render_msg>(job_msg->client,
                                        job_msg->type,
                                        job.job_number,
                                        job.frame_number,
                                        job.chunk,
                                        job.num_chunks,
                                        job.offset_x,
                                        job.offset_y,
                                        job.last_frame,
                                        false,
                                        job.width,
                                        job.height,
                                        transfer_pixels,
                                        "");
  }
}
