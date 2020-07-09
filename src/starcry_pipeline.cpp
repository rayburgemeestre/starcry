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

starcry_pipeline::starcry_pipeline()
    : server(std::make_shared<Server>(std::make_shared<PrintfLogger>(PrintfLogger::Level::Info))),
      chat_handler(std::make_shared<Handler>(this)),
      bitmap(std::make_shared<bitmap_wrapper>()),
      gen(nullptr),
      engine(std::make_shared<rendering_engine_wrapper>()),
      system(std::make_shared<pipeline_system>(false)),
      cmds(system->create_queue("commands", 10)),
      jobs(system->create_queue("jobs", 10)),
      frames(system->create_queue("frames", 10)) {
  engine->initialize();

  // doing this fixes a bunch of segfaults after the optimizer does his thing
  static auto pipeline = this;

  system->spawn_transformer<instruction>(
      "generator",
      [&](auto cmd_def) -> auto {
        pipeline->gen = std::make_shared<generator>([](size_t, int, int, int) {},
                                                    [&](const data::job &job) {
                                                      if (cmd_def->frame != job.frame_number) {
                                                        return true;  // fast forward
                                                      }
                                                      // copied here
                                                      pipeline->the_job = std::make_shared<data::job>(job);
                                                      return false;
                                                    });
        pipeline->gen->init("input/test.js");
        while (pipeline->gen->generate_frame()) {
        }
        return std::make_shared<job_message>(cmd_def->client, pipeline->the_job);
      },
      cmds,
      jobs);

  system->spawn_transformer<job_message>(
      "renderer",
      [&](auto job_msg) -> auto {
        auto &job = job_msg->job;
        png::image<png::rgb_pixel> image(job->width, job->height);
        auto bmp = pipeline->bitmap->get(job->width, job->height);
        render_job(*pipeline->engine, *job, bmp);
        auto pixels = pipeline->engine->serialize_bitmap2(bmp, job->width, job->height);
        copy_to_png(pixels, job->width, job->height, image);
        std::ostringstream ss;
        image.write_stream(ss);
        return std::make_shared<render_msg>(job_msg->client, ss.str());
      },
      jobs,
      frames);

  static auto hand = chat_handler;

  auto xyz = [&](std::shared_ptr<render_msg> job_msg) {
    hand->callback(job_msg->client, job_msg->buffer);
  };

  system->spawn_consumer<render_msg>(
      "** client **",
      [&](std::shared_ptr<render_msg> job_msg) {
        pipeline->server->execute(std::bind(xyz, job_msg));
      },
      frames);

  auto root = std::make_shared<RootPageHandler>();
  root->add(std::make_shared<seasocks::PathHandler>("data", std::make_shared<DataHandler>()));
  server->addPageHandler(root);
  server->addWebSocketHandler("/chat", chat_handler);

  system->start(false);  // non blocking
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
