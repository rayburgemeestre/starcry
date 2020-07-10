/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry_pipeline.h"

#include <generator.h>
#include <render_server.h>
#include <rendering_engine_wrapper.h>
#include <bitmap_wrapper.hpp>
#include <cstring>
#include <sstream>

#include "framer.hpp"
#include "streamer_output/sfml_window.h"

using namespace seasocks;

struct Handler : WebSocket::Handler {
  starcry_pipeline *sc;
  std::set<WebSocket *> _cons;

  explicit Handler(starcry_pipeline *sc) : sc(sc) {}

  void onConnect(WebSocket *con) override {
    _cons.insert(con);
  }
  void onDisconnect(WebSocket *con) override {
    _cons.erase(con);
  }

  void onData(WebSocket *con, const char *data) override {
    sc->add_command(con, "input/test.js", std::atoi(data));
  }

  void callback(WebSocket *recipient, std::string s) {
    if (_cons.find(recipient) != _cons.end()) {
      recipient->send((const uint8_t *)s.c_str(), s.size() * sizeof(uint8_t));
    }
  }
};

struct DataHandler : CrackedUriPageHandler {
  virtual std::shared_ptr<Response> handle(const CrackedUri & /*uri*/, const Request &request) override {
    return Response::jsonResponse("{}");
  }
};

starcry_pipeline::starcry_pipeline(size_t num_local_engines,
                                   bool enable_remote_workers,
                                   bool visualization_enabled,
                                   bool is_interactive,
                                   starcry::render_video_mode mode,
                                   std::function<void(starcry_pipeline &sc)> on_pipeline_initialized)
    : server(std::make_shared<Server>(std::make_shared<PrintfLogger>(PrintfLogger::Level::Info))),
      chat_handler(std::make_shared<Handler>(this)),
      bitmaps({}),
      gen(nullptr),
      engines({}),
      system(std::make_shared<pipeline_system>(visualization_enabled)),
      cmds(system->create_queue("commands", 10)),
      jobs(system->create_queue("jobs", 10)),
      frames(system->create_queue("frames", 10)),
      mode(mode) {
  std::shared_ptr<sfml_window> gui = nullptr;
  if (mode == starcry::render_video_mode::video_with_gui || mode == starcry::render_video_mode::gui_only)
    gui = std::make_unique<sfml_window>();

  std::shared_ptr<frame_streamer> framer = nullptr;
  system->spawn_consumer<instruction>(
      [&](auto cmd_def) {
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
          if (!framer &&
              (mode == starcry::render_video_mode::video_only || mode == starcry::render_video_mode::video_with_gui))
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
      },
      cmds);

  //  if (enable_remote_workers) {
  //    server = std::make_unique<render_server>();
  //    system->spawn_transformer<job_message>(
  //        "render server",
  //        [this](auto job_msg) -> auto {
  //          .... TODO ...
  //          return std::make_shared<render_msg>(job_msg->client, "");
  //        },
  //        jobs,
  //        frames);
  //  }

  for (size_t i = 0; i < num_local_engines; i++) {
    engines[i] = std::make_shared<rendering_engine_wrapper>();
    engines[i]->initialize();
    bitmaps[i] = std::make_shared<bitmap_wrapper>();
    auto foo = [ i = i, this ](auto job_msg) -> auto {
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
    };
    system->spawn_transformer<job_message>("renderer " + std::to_string(i), foo, jobs, frames);
  }

  system->spawn_consumer<render_msg>(
      "** client **",
      [this, &gui, &framer](std::shared_ptr<render_msg> job_msg) {
        if (job_msg->client == nullptr) {
          if (gui) gui->add_frame(job_msg->width, job_msg->height, job_msg->pixels);
          if (framer) framer->add_frame(job_msg->pixels);
        } else {
          server->execute(std::bind(
              [&](std::shared_ptr<render_msg> job_msg) {
                chat_handler->callback(job_msg->client, job_msg->buffer);
              },
              job_msg));
        }
      },
      frames);

  auto root = std::make_shared<RootPageHandler>();
  root->add(std::make_shared<seasocks::PathHandler>("data", std::make_shared<DataHandler>()));
  if (is_interactive) {
    server->addPageHandler(root);
    server->addWebSocketHandler("/chat", chat_handler);
  }
  system->start(false);
  on_pipeline_initialized(*this);
  if (is_interactive) {
    server->serve("webroot", 18080);
  }
  system->explicit_join();
  if (gui) gui->finalize();
  if (framer) framer->finalize();
}

void starcry_pipeline::add_command(WebSocket *client, const std::string &script, int frame_num) {
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
