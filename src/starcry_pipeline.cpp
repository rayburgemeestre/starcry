/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry_pipeline.h"

#include <generator.h>
#include <rendering_engine_wrapper.h>
#include <bitmap_wrapper.hpp>
#include <cstring>
#include <sstream>

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
    sc->add_command(con, std::atoi(data));
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

starcry_pipeline::starcry_pipeline(size_t num_local_engines)
    : server(std::make_shared<Server>(std::make_shared<PrintfLogger>(PrintfLogger::Level::Info))),
      chat_handler(std::make_shared<Handler>(this)),
      bitmaps({}),
      gen(nullptr),
      engines({}),
      system(std::make_shared<pipeline_system>(true)),
      cmds(system->create_queue("commands", 10)),
      jobs(system->create_queue("jobs", 10)),
      frames(system->create_queue("frames", 10)) {

  system->spawn_transformer<instruction>(
      [&](auto cmd_def) -> auto {
        std::shared_ptr<data::job> the_job;
        gen = std::make_shared<generator>([](size_t, int, int, int) {},
                                          [&](const data::job &job) {
                                            if (cmd_def->frame != job.frame_number) {
                                              return true;  // fast forward
                                            }
                                            // copy the job here
                                            the_job = std::make_shared<data::job>(job);
                                            return false;
                                          });
        gen->init("input/test.js");
        while (gen->generate_frame()) {
        }
        return std::make_shared<job_message>(cmd_def->client, the_job);
      },
      cmds,
      jobs,
      transform_type::same_pool);

  for (size_t i = 0; i < num_local_engines; i++) {
    engines[i] = std::make_shared<rendering_engine_wrapper>();
    engines[i]->initialize();
    bitmaps[i] = std::make_shared<bitmap_wrapper>();
    auto foo = [ i = i, this ](auto job_msg) -> auto {
      auto &job = *job_msg->job;
      png::image<png::rgb_pixel> image(job.width, job.height);
      auto bmp = bitmaps[i]->get(job.width, job.height);
      render_job(*engines[i], job, bmp);
      auto pixels = engines[i]->serialize_bitmap2(bmp, job.width, job.height);
      copy_to_png(pixels, job.width, job.height, image);
      std::ostringstream ss;
      image.write_stream(ss);
      return std::make_shared<render_msg>(job_msg->client, ss.str());
    };
    system->spawn_transformer<job_message>("renderer " + std::to_string(i), foo, jobs, frames);
  }

  system->spawn_consumer<render_msg>(
    "** client **",
    [this](std::shared_ptr<render_msg> job_msg) {
      server->execute(std::bind([&](std::shared_ptr<render_msg> job_msg) {
        chat_handler->callback(job_msg->client, job_msg->buffer);
      }, job_msg));
    },
    frames);

  auto root = std::make_shared<RootPageHandler>();
  root->add(std::make_shared<seasocks::PathHandler>("data", std::make_shared<DataHandler>()));
  server->addPageHandler(root);
  server->addWebSocketHandler("/chat", chat_handler);
  system->start(false);
  server->serve("webroot", 18080);
  system->explicit_join();
}

void starcry_pipeline::add_command(WebSocket *client, int frame_num) {
  cmds->push(std::make_shared<instruction>(client, instruction_type::get_image, frame_num));
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
