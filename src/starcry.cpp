/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry.h"

#include <cstring>
#include <sstream>

#include <Magick++.h>
#include <coz.h>
#include <fmt/core.h>

#include "bitmap_wrapper.hpp"
#include "framer.hpp"
#include "generator.h"
#include "network/render_client.h"
#include "network/render_server.h"
#include "rendering_engine_wrapper.h"
#include "streamer_output/sfml_window.h"
#include "util/compress_vector.h"
#include "util/progress_visualizer.h"
#include "webserver.h"

#include "starcry/command_get_bitmap.h"
#include "starcry/command_get_image.h"
#include "starcry/command_get_objects.h"
#include "starcry/command_get_shapes.h"
#include "starcry/command_get_video.h"

#include "starcry/client_message_handler.h"
#include "starcry/server_message_handler.h"

// TODO: re-organize this somehow
#include <random>
std::mt19937 mt_vx;
double rand_fun_vx() {
  return (mt_vx() / (double)mt_vx.max());
}

starcry::starcry(size_t num_local_engines,
                 bool enable_remote_workers,
                 log_level level,
                 bool is_interactive,
                 bool start_webserver,
                 bool enable_compression,
                 starcry::render_video_mode mode,
                 std::function<void(starcry &sc)> on_pipeline_initialized,
                 std::optional<double> rand_seed)
    : num_local_engines(num_local_engines),
      enable_remote_workers(enable_remote_workers),
      is_interactive(is_interactive),
      start_webserver(start_webserver),
      enable_compression(enable_compression),
      bitmaps({}),
      gen(nullptr),
      engines({}),
      system(std::make_shared<pipeline_system>(false)),
      cmds(system->create_queue("commands", 10)),
      jobs(system->create_queue("jobs", 10)),
      frames(system->create_queue("frames", 10)),
      mode(mode),
      on_pipeline_initialized(on_pipeline_initialized),
      le(std::chrono::milliseconds(1000)),
      seed(rand_seed),
      visualizer(std::make_shared<progress_visualizer>()),
      command_handlers({
          {instruction_type::get_bitmap, std::make_shared<command_get_bitmap>(*this)},
          {instruction_type::get_image, std::make_shared<command_get_image>(*this)},
          {instruction_type::get_objects, std::make_shared<command_get_objects>(*this)},
          {instruction_type::get_shapes, std::make_shared<command_get_shapes>(*this)},
          {instruction_type::get_video, std::make_shared<command_get_video>(*this)},
      }),
      server_message_handler_(std::make_shared<server_message_handler>(*this)),
      client_message_handler_(std::make_shared<client_message_handler>(*this)),
      log_level_(level) {}

starcry::~starcry() {
  le.cancel();
}

void starcry::add_command(seasocks::WebSocket *client, const std::string &script, instruction_type it, int frame_num) {
  cmds->push(std::make_shared<instruction>(client, it, script, frame_num));
  le.run([=]() {
    if (webserv) {
      webserv->send_stats(system->stats());
    }
  });
}

void starcry::add_command(seasocks::WebSocket *client, const std::string &script, const std::string &output_file) {
  cmds->push(std::make_shared<instruction>(client, instruction_type::get_video, script, output_file));
}

void starcry::render_job(rendering_engine_wrapper &engine, const data::job &job, image &bmp) {
  data::settings settings = gen->settings();
  engine.render(bmp,
                job.background_color,
                job.shapes,
                job.offset_x,
                job.offset_y,
                job.canvas_w,
                job.canvas_h,
                job.width,
                job.height,
                job.scale,
                job.scales,
                log_level_ == starcry::log_level::debug,
                settings);
  if (job.job_number == std::numeric_limits<uint32_t>::max()) {
    auto filename = gen->filename();
    auto pos = filename.rfind("/");
    if (pos != std::string::npos) {
      filename = filename.substr(pos + 1);
    }
    pos = filename.find(".js");
    if (pos != std::string::npos) {
      filename = filename.substr(0, pos);
    }

    // There is 16 BIT, also + Alpha, however, seems to internally still use an 8 bit palette somehow.
    // Will need to figure out in the future how to properly use 16 bit, for now, will focus on fixing the 8 bit
    // version. png::image<png::rgb_pixel_16> image(job.width, job.height);
    png::image<png::rgb_pixel> image(job.width, job.height);
    copy_to_png(bmp.pixels(), job.width, job.height, image);
    image.write(fmt::format("output_frame_{}_seed_{}_{}x{}-{}.png",
                            job.frame_number,
                            gen->get_seed(),
                            job.canvas_w,
                            job.canvas_h,
                            filename));
    using namespace Magick;
    try {
      std::vector<double> rgb;
      rgb.resize(job.width * job.height * 4);
      size_t index = 0;
      auto &source = bmp.pixels();
      for (uint32_t y = 0; y < job.height; y++) {
        for (uint32_t x = 0; x < job.width; x++) {
          rgb.push_back(source[index].r);
          rgb.push_back(source[index].g);
          rgb.push_back(source[index].b);
          rgb.push_back(source[index].a);
          index++;
        }
      }
      Image image(fmt::format("{}x{}", job.width, job.height).c_str(), "white");
      image.read(job.width, job.height, "RGBA", StorageType::DoublePixel, (void *)&bmp.pixels()[0]);
      //      image.read(Blob((void *) &bmp.pixels()[0], bmp.pixels().size()), Geometry(job.width, job.height));
      // image.depth(32);
      image.write(fmt::format("output_frame_{}_seed_{}_{}x{}-{}.exr",
                              job.frame_number,
                              gen->get_seed(),
                              job.canvas_w,
                              job.canvas_h,
                              filename));
    } catch (std::exception &error_) {
      std::cout << "Caught exception: " << error_.what() << std::endl;
    }
  }
}

void starcry::copy_to_png(const std::vector<data::color> &source,
                          uint32_t width,
                          uint32_t height,
                          png::image<png::rgb_pixel> &dest) {
  size_t index = 0;
  auto m = std::numeric_limits<uint8_t>::max();
  auto settings = gen->settings();
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      // BGRA -> RGB
      // uint8_t *data = (uint8_t *)&(source[index]);
      // dest[y][x] = png::rgb_pixel(*(data + 2), *(data + 1), *(data + 0));

      // My attempt at "random" dithering, probably inefficient
      double r1 = source[index].r;
      double g1 = source[index].g;
      double b1 = source[index].b;
      uint8_t r = r1 * m;
      uint8_t g = g1 * m;
      uint8_t b = b1 * m;
      if (settings.dithering) {
        double r_dbl = r1 * m;
        double g_dbl = g1 * m;
        double b_dbl = b1 * m;
        double r_offset = r_dbl - static_cast<double>(r);
        double g_offset = g_dbl - static_cast<double>(g);
        double b_offset = b_dbl - static_cast<double>(b);
        if (rand_fun_vx() >= r_offset && r > 0) {
          r -= 1;
        }
        if (rand_fun_vx() >= g_offset && g > 0) {
          g -= 1;
        }
        if (rand_fun_vx() >= b_offset && b > 0) {
          b -= 1;
        }
      }
      dest[y][x] = png::rgb_pixel(r, g, b);
      index++;
    }
  }
}

void starcry::command_to_jobs(std::shared_ptr<instruction> cmd_def) {
  if (!gen) gen = std::make_shared<generator>();
  gen->init(cmd_def->script, seed);

  if (cmd_def->type == instruction_type::get_video) {
    command_handlers[cmd_def->type]->to_job(cmd_def);
  } else {
    command_handlers[instruction_type::get_bitmap]->to_job(cmd_def);
  }
  if (cmd_def->client == nullptr) {
    cmds->check_terminate();
    jobs->check_terminate();
  }
}

std::shared_ptr<render_msg> starcry::job_to_frame(size_t i, std::shared_ptr<job_message> job_msg) {
  auto &job = *job_msg->job;
  visualizer->set_start_timing();

  if (mode == starcry::render_video_mode::javascript_only) {
    std::vector<uint32_t> transfer_pixels;
    return std::make_shared<render_msg>(
        job_msg->client, job_msg->type, job.job_number, job.last_frame, 0, 0, transfer_pixels);
  }

  // no need to render in this case, client will do the rendering
  if (job_msg->type == instruction_type::get_shapes) {
    image empty;
    return command_handlers[job_msg->type]->to_render_msg(job_msg, empty);
  }
  if (job_msg->type == instruction_type::get_objects) {
    image empty;
    return command_handlers[job_msg->type]->to_render_msg(job_msg, empty);
  }

  // render
  auto &bmp = bitmaps[i]->get(job.width, job.height);
  render_job(*engines[i], job, bmp);

  return command_handlers[job_msg->type]->to_render_msg(job_msg, bmp);
}

void starcry::handle_frame(std::shared_ptr<render_msg> job_msg) {
  visualizer->display(job_msg->job_number);

  if (mode == starcry::render_video_mode::javascript_only) {
    return;
  }
  // COZ_PROGRESS;
  auto process = [&](std::shared_ptr<render_msg> job_msg) {
    if (job_msg->client == nullptr) {
      if (gui) gui->add_frame(job_msg->width, job_msg->height, job_msg->pixels);
      if (framer) {
        framer->add_frame(job_msg->pixels);
        if (job_msg->last_frame) {
          for (int i = 0; i < 50; i++) {
            framer->add_frame(job_msg->pixels);
          }
        }
      }
      return;
    }
    command_handlers[job_msg->type]->handle_frame(job_msg);
  };

  // handle individual frames immediately
  if (job_msg->job_number == std::numeric_limits<uint32_t>::max()) {
    process(job_msg);
    return;
  }

  buffered_frames[job_msg->job_number] = job_msg;

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

  system->spawn_consumer<instruction>(
      "generator", std::bind(&starcry::command_to_jobs, this, std::placeholders::_1), cmds);

  if (enable_remote_workers) {
    renderserver = std::make_shared<render_server>(jobs, frames);
    renderserver->run(std::bind(&client_message_handler::on_client_message,
                                *client_message_handler_,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3,
                                std::placeholders::_4));
  }

  for (size_t i = 0; i < num_local_engines; i++) {
    engines[i] = std::make_shared<rendering_engine_wrapper>();
    engines[i]->initialize();
    bitmaps[i] = std::make_shared<bitmap_wrapper>();
    system->spawn_transformer<job_message>("renderer-" + std::to_string(i),
                                           std::bind(&starcry::job_to_frame, this, i, std::placeholders::_1),
                                           jobs,
                                           frames,
                                           transform_type::same_pool);
  }

  system->spawn_consumer<render_msg>(
      "streamer", std::bind(&starcry::handle_frame, this, std::placeholders::_1), frames);

  system->start(false);
  on_pipeline_initialized(*this);
  if (start_webserver) {
    webserv = std::make_shared<webserver>(this);
    webserv->run();  // blocks
  }
  system->explicit_join();
  if (gui) gui->finalize();
  if (framer) framer->finalize();
  std::cout << std::endl;
}

void starcry::run_client(const std::string &host) {
  render_client client(host);
  rendering_engine_wrapper engine;
  bitmap_wrapper bitmap;
  engine.initialize();

  client.set_message_fun([&](int fd, int type, size_t len, const std::string &msg) {
    server_message_handler_->on_server_message(client, engine, bitmap, fd, type, len, msg);
  });

  client.register_me();
  while (client.poll())
    ;
}

// This function is used to send to framer and als the preview window
std::vector<uint32_t> starcry::pixels_vec_to_pixel_data(const std::vector<data::color> &pixels_in) const {
  std::vector<uint32_t> pixels_out;
  pixels_out.reserve(pixels_in.size());

  if (gen->settings().dithering) {
    for (const auto &pix : pixels_in) {
      uint32_t color;
      char *cptr = (char *)&color;
      auto r = (char)(pix.r * 255);
      auto g = (char)(pix.g * 255);
      auto b = (char)(pix.b * 255);
      auto a = (char)(pix.a * 255);
      double r_dbl = pix.r * 255.;
      double g_dbl = pix.g * 255.;
      double b_dbl = pix.b * 255.;
      double a_dbl = pix.a * 255.;
      double r_offset = r_dbl - static_cast<double>(r);
      double g_offset = g_dbl - static_cast<double>(g);
      double b_offset = b_dbl - static_cast<double>(b);
      double a_offset = a_dbl - static_cast<double>(a);
      if (rand_fun_vx() >= r_offset && r > 0) {
        r -= 1;
      }
      if (rand_fun_vx() >= g_offset && g > 0) {
        g -= 1;
      }
      if (rand_fun_vx() >= b_offset && b > 0) {
        b -= 1;
      }
      if (rand_fun_vx() >= a_offset && a > 0) {
        a -= 1;
      }
      *cptr = r;
      cptr++;
      *cptr = g;
      cptr++;
      *cptr = b;
      cptr++;
      *cptr = a;
      pixels_out.push_back(color);
    }
  } else {
    for (const auto &pix : pixels_in) {
      uint32_t color;
      char *cptr = (char *)&color;
      *cptr = (char)(pix.r * 255);
      cptr++;
      *cptr = (char)(pix.g * 255);
      cptr++;
      *cptr = (char)(pix.b * 255);
      cptr++;
      *cptr = (char)(pix.a * 255);
      pixels_out.push_back(color);
    }
  }
  return pixels_out;
}
