/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry_pipeline.h"

#include <cstring>
#include <sstream>

#include "cereal/archives/binary.hpp"
#include "framer.hpp"
#include "generator.h"
#include "render_server.h"
#include "rendering_engine_wrapper.h"
#include "bitmap_wrapper.hpp"
#include "streamer_output/sfml_window.h"
#include "webserver.h"

starcry_pipeline::starcry_pipeline(size_t num_local_engines,
                                   bool enable_remote_workers,
                                   bool visualization_enabled,
                                   bool is_interactive,
                                   starcry_pipeline::render_video_mode mode,
                                   std::function<void(starcry_pipeline &sc)> on_pipeline_initialized)
    : bitmaps({}),
      gen(nullptr),
      engines({}),
      system(std::make_shared<pipeline_system>(visualization_enabled)),
      cmds(system->create_queue("commands", 10)),
      jobs(system->create_queue("jobs", 10)),
      frames(system->create_queue("frames", 10)),
      mode(mode) {
  if (mode == starcry_pipeline::render_video_mode::video_with_gui ||
      mode == starcry_pipeline::render_video_mode::gui_only)
    gui = std::make_unique<sfml_window>();

  system->spawn_consumer<instruction>(std::bind(&starcry_pipeline::command_to_jobs, this, std::placeholders::_1), cmds);

  if (enable_remote_workers) {
    renderserver = std::make_shared<render_server>(jobs, frames);
    renderserver->run(std::bind(&starcry_pipeline::on_client_message,
                                this,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3,
                                std::placeholders::_4));
  }

  for (size_t i = 0; i < num_local_engines; i++) {
    engines[i] = std::make_shared<rendering_engine_wrapper>();
    engines[i]->initialize();
    bitmaps[i] = std::make_shared<bitmap_wrapper>();
    system->spawn_transformer<job_message>("renderer " + std::to_string(i),
                                           std::bind(&starcry_pipeline::job_to_frame, this, i, std::placeholders::_1),
                                           jobs,
                                           frames,
                                           transform_type::same_pool);
  }

  system->spawn_consumer<render_msg>(
      "** client **", std::bind(&starcry_pipeline::handle_frame, this, std::placeholders::_1), frames);

  system->start(false);
  on_pipeline_initialized(*this);
  if (is_interactive) {
    webserv = std::make_shared<webserver>(this);  // blocks
    webserv->run();
  }
  system->explicit_join();
  if (gui) gui->finalize();
  if (framer) framer->finalize();
}

void starcry_pipeline::add_command(seasocks::WebSocket *client, const std::string &script, int frame_num) {
  cmds->push(std::make_shared<instruction>(client, instruction_type::get_image, script, frame_num));
}

void starcry_pipeline::add_command(seasocks::WebSocket *client,
                                   const std::string &script,
                                   const std::string &output_file) {
  cmds->push(std::make_shared<instruction>(client, instruction_type::get_video, script, output_file));
}

void starcry_pipeline::render_job(rendering_engine_wrapper &engine, const data::job &job, ALLEGRO_BITMAP *bmp) {
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

void starcry_pipeline::copy_to_png(const std::vector<uint32_t> &source,
                                   uint32_t width,
                                   uint32_t height,
                                   png::image<png::rgb_pixel> &dest) {
  size_t index = 0;
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      // BGRA -> RGB
      uint8_t *data = (uint8_t *)&(source[index]);
      dest[y][x] = png::rgb_pixel(*(data + 2), *(data + 1), *(data + 0));
      index++;
    }
  }
}
void starcry_pipeline::command_to_jobs(std::shared_ptr<instruction> cmd_def) {
  std::shared_ptr<data::job> the_job;
  if (cmd_def->type == instruction_type::get_image) {
    gen = std::make_shared<generator>([](size_t, int, int, int) {},
                                      [&](const data::job &job) {
                                        if (cmd_def->frame != job.frame_number) {
                                          return true;  // fast forward
                                        }
                                        // copy the job here
                                        the_job = std::make_shared<data::job>(job);
                                        return false;
                                      });
    gen->init(cmd_def->script);
    while (gen->generate_frame()) {
    }
    jobs->push(std::make_shared<job_message>(cmd_def->client, the_job));
    jobs->sleep_until_not_full();
  } else if (cmd_def->type == instruction_type::get_video) {
    if (!framer && (mode == starcry_pipeline::render_video_mode::video_only ||
                    mode == starcry_pipeline::render_video_mode::video_with_gui))
      framer = std::make_unique<frame_streamer>(cmd_def->output_file, frame_streamer::stream_mode::FILE);
    gen = std::make_shared<generator>(
        [&](size_t bitrate, int width, int height, int fps) {
          if (framer) framer->initialize(bitrate, width, height, fps /*use_fps ? *use_fps : fps*/);
        },
        [&](const data::job &job) {
          the_job = std::make_shared<data::job>(job);
          return true;
        });
    gen->init(cmd_def->script);
    while (true) {
      auto ret = gen->generate_frame();
      jobs->push(std::make_shared<job_message>(cmd_def->client, the_job));
      jobs->sleep_until_not_full();
      if (!ret) break;
    }
    // bring the house down
    cmds->check_terminate();
    jobs->check_terminate();
  }
}

bool starcry_pipeline::on_client_message(int sockfd, int type, size_t len, const std::string &data) {
  switch (type) {
    case 10:  // register_me
    {
      renderserver->send_msg(sockfd, type + 1, (const char *)&num_queue_per_worker, sizeof(num_queue_per_worker));
      break;
    }
    case 20:  // pull_job
    {
      while (true) {
        if (!jobs->has_items(0)) {
          jobs->sleep_until_items_available(0);
          if (!jobs->active) {
            frames->check_terminate();
            return false;  // shutdown server
          }
        }
        auto job = std::dynamic_pointer_cast<job_message>(jobs->pop(0));
        if (job) {  // can be nullptr if someone else took it faster than us
          std::ostringstream os;
          cereal::BinaryOutputArchive archive(os);
          archive(*(job->job));
          renderserver->send_msg(sockfd, type + 1, os.str().c_str(), os.str().size());
          break;
        }
      }
      break;
    }
    case 30: {
      std::istringstream is(data);
      cereal::BinaryInputArchive archive(is);
      data::job job;
      data::pixel_data2 dat;
      bool is_remote;
      archive(job);
      archive(dat);
      archive(is_remote);
      auto frame = std::make_shared<render_msg>(nullptr, job.width, job.height, dat.pixels);
      frames->push(frame);
    }
  }
  return true;
}

std::shared_ptr<render_msg> starcry_pipeline::job_to_frame(size_t i, std::shared_ptr<job_message> job_msg) {
  auto &job = *job_msg->job;
  png::image<png::rgb_pixel> image(job.width, job.height);
  auto bmp = bitmaps[i]->get(job.width, job.height);
  render_job(*engines[i], job, bmp);
  auto pixels = engines[i]->serialize_bitmap2(bmp, job.width, job.height);  // rvo
  if (job_msg->client == nullptr) {
    return std::make_shared<render_msg>(job_msg->client, job.width, job.height, pixels);
  } else {
    copy_to_png(pixels, job.width, job.height, image);
    std::ostringstream ss;
    image.write_stream(ss);
    return std::make_shared<render_msg>(job_msg->client, job.width, job.height, ss.str());
  }
}

void starcry_pipeline::handle_frame(std::shared_ptr<render_msg> job_msg) {
  if (job_msg->client == nullptr) {
    if (gui) gui->add_frame(job_msg->width, job_msg->height, job_msg->pixels);
    if (framer) framer->add_frame(job_msg->pixels);
  } else {
    auto fun = [&](std::shared_ptr<Handler> chat_handler, std::shared_ptr<render_msg> job_msg) {
      chat_handler->callback(job_msg->client, job_msg->buffer);
    };
    if (webserv) webserv->execute(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
  }
}
