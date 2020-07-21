/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry.h"

#include <cstring>
#include <sstream>

#include "bitmap_wrapper.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "framer.hpp"
#include "generator.h"
#include "render_client.h"
#include "render_server.h"
#include "rendering_engine_wrapper.h"
#include "streamer_output/sfml_window.h"
#include "util/compress_vector.h"
#include "webserver.h"

starcry::starcry(size_t num_local_engines,
                 bool enable_remote_workers,
                 bool visualization_enabled,
                 bool is_interactive,
                 bool start_webserver,
                 bool enable_compression,
                 starcry::render_video_mode mode,
                 std::function<void(starcry &sc)> on_pipeline_initialized)
    : num_local_engines(num_local_engines),
      enable_remote_workers(enable_remote_workers),
      is_interactive(is_interactive),
      start_webserver(start_webserver),
      enable_compression(enable_compression),
      bitmaps({}),
      gen(nullptr),
      engines({}),
      system(std::make_shared<pipeline_system>(visualization_enabled)),
      cmds(system->create_queue("commands", 10)),
      jobs(system->create_queue("jobs", 10)),
      frames(system->create_queue("frames", 10)),
      mode(mode),
      on_pipeline_initialized(on_pipeline_initialized) {}

void starcry::add_command(seasocks::WebSocket *client, const std::string &script, instruction_type it, int frame_num) {
  cmds->push(std::make_shared<instruction>(client, it, script, frame_num));
}

void starcry::add_command(seasocks::WebSocket *client, const std::string &script, const std::string &output_file) {
  cmds->push(std::make_shared<instruction>(client, instruction_type::get_video, script, output_file));
}

void starcry::render_job(rendering_engine_wrapper &engine, const data::job &job, image &bmp) {
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

void starcry::copy_to_png(const std::vector<data::color> &source,
                          uint32_t width,
                          uint32_t height,
                          png::image<png::rgb_pixel> &dest) {
  size_t index = 0;
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      // BGRA -> RGB
      // uint8_t *data = (uint8_t *)&(source[index]);
      // dest[y][x] = png::rgb_pixel(*(data + 2), *(data + 1), *(data + 0));
      dest[y][x] = png::rgb_pixel(source[index].r * 255, source[index].g * 255, source[index].b * 255);
      index++;
    }
  }
}
void starcry::command_to_jobs(std::shared_ptr<instruction> cmd_def) {
  std::shared_ptr<data::job> the_job;
  if (cmd_def->type == instruction_type::get_image || cmd_def->type == instruction_type::get_shapes) {
    gen = std::make_shared<generator>([](size_t, int, int, int) {},
                                      [&](const data::job &job) {
                                        if (cmd_def->frame != job.frame_number) {
                                          return true;  // fast forward
                                        }
                                        // copy the job here
                                        the_job = std::make_shared<data::job>(job);
                                        if (enable_compression) {
                                          the_job->compress = true;
                                        }
                                        return false;
                                      });
    gen->init(cmd_def->script);
    while (gen->generate_frame()) {
    }
    the_job->job_number = std::numeric_limits<uint32_t>::max();
    jobs->push(std::make_shared<job_message>(cmd_def->client, cmd_def->type, the_job));
    if (cmd_def->client == nullptr) {  // cli thing
      cmds->check_terminate();
      jobs->check_terminate();
    }
    jobs->sleep_until_not_full();
  } else if (cmd_def->type == instruction_type::get_video) {
    std::optional<int> use_fps;
    if (!framer &&
        (mode == starcry::render_video_mode::video_only || mode == starcry::render_video_mode::video_with_gui) &&
        cmd_def->output_file != "/dev/null") {
      auto stream_mode = frame_streamer::stream_mode::FILE;
      auto output_file = cmd_def->output_file;
      if (output_file.substr(output_file.size() - 4, 4) == "m3u8") {
        use_fps = 1000;
        stream_mode = frame_streamer::stream_mode::HLS;
      }
      framer = std::make_unique<frame_streamer>(cmd_def->output_file, stream_mode);
    }

    gen = std::make_shared<generator>(
        [&](size_t bitrate, int width, int height, int fps) {
          if (framer) framer->initialize(bitrate, width, height, use_fps ? *use_fps : fps);
        },
        [&](const data::job &job) {
          the_job = std::make_shared<data::job>(job);
          return true;
        });
    gen->init(cmd_def->script);
    while (true) {
      auto ret = gen->generate_frame();
      jobs->push(std::make_shared<job_message>(cmd_def->client, cmd_def->type, the_job));
      jobs->sleep_until_not_full();
      if (!ret) break;
    }
    // bring the house down
    cmds->check_terminate();
    jobs->check_terminate();
  }
}

bool starcry::on_client_message(int sockfd, int type, size_t len, const std::string &data) {
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

      // if (job.compress) {
      //   compress_vector<uint32_t> cv;
      //   cv.decompress(&dat.pixels, job.width * job.height);
      // }

      auto frame = std::make_shared<render_msg>(
          nullptr, instruction_type::get_image, job.job_number, job.width, job.height, dat.pixels);
      frames->push(frame);
    }
  }
  return true;
}

std::shared_ptr<render_msg> starcry::job_to_frame(size_t i, std::shared_ptr<job_message> job_msg) {
  auto &job = *job_msg->job;
  // no need to render in this case, client will do the rendering
  if (job_msg->type == instruction_type::get_shapes) {
    std::ostringstream os;
    {
      // cereal::BinaryOutputArchive archive(os);
      cereal::JSONOutputArchive archive(os);
      archive(job);
    }
    return std::make_shared<render_msg>(
        job_msg->client, job_msg->type, job.job_number, job.width, job.height, os.str());
  }

  auto &bmp = bitmaps[i]->get(job.width, job.height);
  render_job(*engines[i], job, bmp);
  if (job.job_number == std::numeric_limits<uint32_t>::max()) {
    engines[i]->write_image(bmp, "output.bmp");
  }
  if (job_msg->client == nullptr) {
    std::vector<uint32_t> transfer_pixels;
    auto &pixels = bmp.pixels();
    transfer_pixels.reserve(pixels.size());
    for (const auto &pix : pixels) {
      uint32_t color;
      char *cptr = (char *)&color;
      *cptr = (char)(pix.r * 255);
      cptr++;
      *cptr = (char)(pix.g * 255);
      cptr++;
      *cptr = (char)(pix.b * 255);
      cptr++;
      *cptr = (char)(pix.a * 255);
      transfer_pixels.push_back(color);
    }

    return std::make_shared<render_msg>(
        job_msg->client, job_msg->type, job.job_number, job.width, job.height, transfer_pixels);
  } else {
    job.job_number = std::numeric_limits<uint32_t>::max();
    png::image<png::rgb_pixel> image(job.width, job.height);
    copy_to_png(bmp.pixels(), job.width, job.height, image);
    std::ostringstream ss;
    image.write_stream(ss);
    return std::make_shared<render_msg>(
        job_msg->client, job_msg->type, job.job_number, job.width, job.height, ss.str());
  }
}

void starcry::handle_frame(std::shared_ptr<render_msg> job_msg) {
  auto process = [&](std::shared_ptr<render_msg> job_msg) {
    if (job_msg->client == nullptr) {
      if (gui) gui->add_frame(job_msg->width, job_msg->height, job_msg->pixels);
      if (framer) {
        framer->add_frame(job_msg->pixels);
      }
    } else {
      if (job_msg->type == instruction_type::get_image) {
        auto fun = [&](std::shared_ptr<ImageHandler> chat_handler, std::shared_ptr<render_msg> job_msg) {
          chat_handler->callback(job_msg->client, job_msg->buffer);
        };
        if (webserv) webserv->execute_image(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
      } else if (job_msg->type == instruction_type::get_shapes) {
        auto fun = [&](std::shared_ptr<ShapesHandler> shapes_handler, std::shared_ptr<render_msg> job_msg) {
          shapes_handler->callback(job_msg->client, job_msg->buffer);
        };
        if (webserv) webserv->execute_shapes(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
      }
    }
  };

  // handle individual frames immediately
  if (job_msg->job_number == std::numeric_limits<uint32_t>::max()) {
    process(job_msg);
    return;
  }

  buffered_frames[job_msg->job_number] = job_msg;

  // std::cout << "current frame: " << current_frame << ", buffer size: " << buffered_frames.size() << std::endl;
  while (true) {
    auto pos = buffered_frames.find(current_frame);
    if (pos == buffered_frames.end()) {
      break;
    }
    process(pos->second);
    current_frame++;
    buffered_frames.erase(pos);
  }
}

void starcry::run_server() {
  if (mode == starcry::render_video_mode::video_with_gui || mode == starcry::render_video_mode::gui_only)
    gui = std::make_unique<sfml_window>();

  system->spawn_consumer<instruction>(std::bind(&starcry::command_to_jobs, this, std::placeholders::_1), cmds);

  if (enable_remote_workers) {
    renderserver = std::make_shared<render_server>(jobs, frames);
    renderserver->run(std::bind(&starcry::on_client_message,
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
                                           std::bind(&starcry::job_to_frame, this, i, std::placeholders::_1),
                                           jobs,
                                           frames,
                                           transform_type::same_pool);
  }

  system->spawn_consumer<render_msg>(
      "** client **", std::bind(&starcry::handle_frame, this, std::placeholders::_1), frames);

  system->start(false);
  on_pipeline_initialized(*this);
  if (start_webserver) {
    webserv = std::make_shared<webserver>(this);  // blocks
    webserv->run();
  }
  system->explicit_join();
  if (gui) gui->finalize();
  if (framer) framer->finalize();
}

void starcry::run_client() {
  rendering_engine_wrapper engine;
  engine.initialize();
  bitmap_wrapper bitmap;
  int64_t num_queue_per_worker = 0;  // currently not used..
  render_client client;

  client.set_message_fun([&](int sockfd, int type, size_t len, const std::string &data) {
    switch (type) {
      case 11:  // register_me response
      {
        if (len == sizeof(num_queue_per_worker)) {
          memcpy(&num_queue_per_worker, data.c_str(), sizeof(num_queue_per_worker));
        }
        client.pull_job(true, 0);
        break;
      }
      case 21:  // pull_job answer
      {
        std::istringstream is(data);
        cereal::BinaryInputArchive archive(is);
        data::job job;
        archive(job);
        auto &bmp = bitmap.get(job.width, job.height);
        render_job(engine, job, bmp);

        data::pixel_data2 dat;
        auto &pixels = bmp.pixels();

        dat.pixels.reserve(pixels.size());
        for (const auto &pix : pixels) {
          uint32_t color;
          char *cptr = (char *)&color;
          *cptr = (char)(pix.r * 255);
          cptr++;
          *cptr = (char)(pix.g * 255);
          cptr++;
          *cptr = (char)(pix.b * 255);
          cptr++;
          *cptr = (char)(pix.a * 255);
          dat.pixels.push_back(color);
        }

        client.send_frame(job, dat, true);
        client.pull_job(true, 0);
        break;
      }
    }
  });
  client.register_me();
  while (client.poll()) {
    std::cout << "client poll.." << std::endl;
  }
}
