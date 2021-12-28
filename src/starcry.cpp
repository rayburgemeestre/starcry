/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry.h"

#include <cstring>
#include <sstream>

#include <coz.h>
#include <fmt/core.h>

#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfStringAttribute.h>

#include "bitmap_wrapper.hpp"
#include "framer.hpp"
#include "generator.h"
#include "network/render_client.h"
#include "network/render_server.h"
#include "rendering_engine_wrapper.h"
#include "streamer_output/sfml_window.h"
#include "util/compress_vector.h"
#include "util/image_utils.h"
#include "util/logger.h"
#include "webserver.h"

#include "starcry/client_message_handler.h"
#include "starcry/command_get_bitmap.h"
#include "starcry/command_get_image.h"
#include "starcry/command_get_objects.h"
#include "starcry/command_get_raw_image.h"
#include "starcry/command_get_shapes.h"
#include "starcry/command_get_video.h"
#include "starcry/metrics.h"
#include "starcry/server_message_handler.h"

// TODO: re-organize this somehow
#include <sys/prctl.h>
#include <random>
#include <utility>

#include <inotify-cpp/FileSystemAdapter.h>
#include <inotify-cpp/NotifierBuilder.h>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::mt19937 mt_vx;
double rand_fun_vx() {
  return (mt_vx() / (double)mt_vx.max());
}

starcry::starcry(size_t num_local_engines,
                 bool enable_remote_workers,
                 log_level level,
                 bool notty,
                 bool start_webserver,
                 bool enable_compression,
                 starcry::render_video_mode mode,
                 std::function<void(starcry &sc)> on_pipeline_initialized,
                 std::optional<double> rand_seed)
    : num_local_engines(num_local_engines),
      enable_remote_workers(enable_remote_workers),
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
      on_pipeline_initialized(std::move(on_pipeline_initialized)),
      pe(std::chrono::milliseconds(1000)),
      seed(rand_seed),
      command_handlers({
          {instruction_type::get_bitmap, std::make_shared<command_get_bitmap>(*this)},
          {instruction_type::get_image, std::make_shared<command_get_image>(*this)},
          {instruction_type::get_objects, std::make_shared<command_get_objects>(*this)},
          {instruction_type::get_shapes, std::make_shared<command_get_shapes>(*this)},
          {instruction_type::get_video, std::make_shared<command_get_video>(*this)},
          {instruction_type::get_raw_image, std::make_shared<command_get_raw_image>(*this)},
      }),
      server_message_handler_(std::make_shared<server_message_handler>(*this)),
      client_message_handler_(std::make_shared<client_message_handler>(*this)),
      log_level_(level),
      metrics_(std::make_shared<metrics>(notty)),
      script_("input/test.js"),
      notifier(nullptr) {
  metrics_->set_script(script_);
  metrics_->init();
  set_metrics(&*metrics_);
  logger(DEBUG) << "Metrics wired to ncurses UI" << std::endl;

  inotifypp::filesystem::path path("input");
  auto handleNotification = [&](inotify::Notification notification) {
    logger(DEBUG) << "File modified on disk: " << notification.path.string() << std::endl;
    if (notification.path.string() == script_) {
      // TODO: for the future implement HOT swapping (requires parsing JSON and merging intelligently)
      gen->reset_context();
      gen->init(script_, {}, viewpoint.preview, features_.caching);
      json j{
          {"type", "fs_change"},
          {"file", script_},
      };
      webserv->send_fs_change(j.dump());
    }
  };
  auto handleUnexpectedNotification = [](inotify::Notification notification) {};
  auto events = {inotify::Event::close_write};
  notifier = std::make_unique<inotify::NotifierBuilder>();
  notifier->watchPathRecursively(path)
      .onEvents(events, handleNotification)
      .onUnexpectedEvent(handleUnexpectedNotification);
  notifier_thread = std::thread([&]() {
    notifier->run();
  });
}

starcry::~starcry() {
  notifier->stop();
  metrics_->notify();
  pe.cancel();
  notifier_thread.join();
}

feature_settings &starcry::features() {
  return features_;
}

void starcry::set_script(const std::string &script) {
  script_ = script;
  if (metrics_) metrics_->set_script(script_);
  if (webserv) webserv->set_script(script_);
}

void starcry::add_command(seasocks::WebSocket *client,
                          const std::string &script,
                          instruction_type it,
                          int frame_num,
                          int num_chunks,
                          bool raw,
                          bool preview,
                          bool last_frame,
                          const std::string &output_filename) {
  cmds->push(std::make_shared<instruction>(
      client, it, script, frame_num, num_chunks, raw, preview, last_frame, output_filename));
  pe.run([=]() {
    if (webserv) {
      webserv->send_stats(system->get_stats());
      webserv->send_metrics(metrics_->to_json());
    }
  });
}

void starcry::add_command(seasocks::WebSocket *client,
                          const std::string &script,
                          const std::string &output_file,
                          int num_chunks,
                          bool raw,
                          bool preview,
                          size_t offset_frames) {
  cmds->push(std::make_shared<instruction>(
      client, instruction_type::get_video, script, output_file, num_chunks, raw, preview, offset_frames));
}

void starcry::render_job(size_t thread_num,
                         rendering_engine_wrapper &engine,
                         const data::job &job,
                         image &bmp,
                         const data::settings &settings) {
  prctl(PR_SET_NAME, fmt::format("sc {} {}/{}", job.frame_number, job.chunk, job.num_chunks).c_str(), NULL, NULL, NULL);

  bmp = engine.render(thread_num,
                      job.job_number == std::numeric_limits<uint32_t>::max() ? job.frame_number : job.job_number,
                      job.chunk,
                      metrics_,
                      job.background_color,
                      job.shapes,
                      job.view_x,
                      job.view_y,
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
}

void starcry::command_to_jobs(std::shared_ptr<instruction> cmd_def) {
  if (!gen) gen = std::make_shared<generator>(metrics_);
  gen->init(cmd_def->script, seed, cmd_def->preview, features_.caching);

  if (cmd_def->type == instruction_type::get_video) {
    // Update current_frame if we fast-forward to a different offset frame.
    if (cmd_def->offset_frames > 0) {
      current_frame = cmd_def->offset_frames;
    }
    command_handlers[cmd_def->type]->to_job(cmd_def);
  } else {
    cmd_def->viewpoint = viewpoint;
    command_handlers[instruction_type::get_bitmap]->to_job(cmd_def);
  }
  if (cmd_def->client == nullptr) {
    cmds->check_terminate();
    jobs->check_terminate();
  }
}

std::shared_ptr<render_msg> starcry::job_to_frame(size_t i, std::shared_ptr<job_message> job_msg) {
  auto &job = *job_msg->job;

  if (mode == starcry::render_video_mode::javascript_only) {
    std::vector<uint32_t> transfer_pixels;
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
                                        0,
                                        0,
                                        transfer_pixels,
                                        job.output_file);
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
  data::settings settings = gen->settings();
  if (job.job_number == std::numeric_limits<uint32_t>::max()) {
    metrics_->set_frame_mode();
    metrics_->render_job(i, job.frame_number, job.chunk);
  } else {
    metrics_->render_job(i, job.job_number, job.chunk);
  }
  render_job(i, *engines[i], job, bmp, settings);

  if (job.job_number == std::numeric_limits<uint32_t>::max()) {
    metrics_->complete_render_job(i, job.frame_number, job.chunk);
  } else {
    metrics_->complete_render_job(i, job.job_number, job.chunk);
  }

  return command_handlers[job_msg->type]->to_render_msg(job_msg, bmp);
}

void starcry::handle_frame(std::shared_ptr<render_msg> job_msg) {
  bool finished = false;
  if (log_level_ != starcry::log_level::silent) {
    const auto frame = job_msg->job_number == std::numeric_limits<size_t>::max() ? 0 : job_msg->job_number;
    static auto prev_frame = 0;
    if (log_level_ != starcry::log_level::info) {
      if (frame != prev_frame) {
      }
    }
    prev_frame = frame;
  }

  if (mode == starcry::render_video_mode::javascript_only) {
    return;
  }
  // COZ_PROGRESS;
  auto process = [&](size_t width,
                     size_t height,
                     std::vector<uint32_t> &pixels,
                     std::vector<data::color> &pixels_raw,
                     bool last_frame) {
    bool finished = false;
    if (job_msg->client == nullptr) {
      // get pixels from raw pixels for the ffmpeg video
      if (pixels.empty() && !pixels_raw.empty()) {
        std::vector<uint32_t> pixels_new = starcry::pixels_vec_to_pixel_data(pixels_raw, gen->settings());
        std::swap(pixels, pixels_new);
      }
      // insert checkers background
      starcry::pixels_vec_insert_checkers_background(pixels, width, height);
      if (gui) {
        gui->add_frame(width, height, pixels);
      }
      if (framer) {
        framer->add_frame(pixels);
        if (last_frame) {
          for (int i = 0; i < 50; i++) {
            framer->add_frame(pixels);
          }
          finished = true;
        }
      }
      return finished;
    }
    std::swap(job_msg->pixels, pixels);
    std::swap(job_msg->pixels_raw, pixels_raw);
    command_handlers[job_msg->type]->handle_frame(job_msg);
    return finished;
  };

  buffered_frames[job_msg->job_number].push_back(job_msg);

  if (job_msg->type == instruction_type::get_shapes || job_msg->type == instruction_type::get_objects) {
    current_frame = job_msg->frame_number;
  }

  while (true) {
    auto pos = buffered_frames.find(current_frame);
    if (pos == buffered_frames.end()) {
      pos = buffered_frames.find(std::numeric_limits<uint32_t>::max());
      if (pos == buffered_frames.end()) {
        break;
      }
    }
    if (pos->second.empty()) {
      break;
    }
    if (pos->second.size() == pos->second[0]->num_chunks) {
      std::sort(pos->second.begin(), pos->second.end(), [](const auto &lh, const auto &rh) {
        return lh->chunk < rh->chunk;
      });

      std::vector<uint32_t> pixels;
      std::vector<data::color> pixels_raw;
      size_t width = 0;
      size_t height = 0;
      bool last_frame = false;
      size_t frame_number = 0;
      for (const auto &chunk : pos->second) {
        pixels.insert(std::end(pixels), std::begin(chunk->pixels), std::end(chunk->pixels));
        pixels_raw.insert(std::end(pixels_raw), std::begin(chunk->pixels_raw), std::end(chunk->pixels_raw));
        width = chunk->width;  // width does not need to be accumulated since we split in horizontal slices
        height += chunk->height;
        last_frame = chunk->last_frame;
        frame_number = chunk->frame_number;
      }

      finished = process(width, height, pixels, pixels_raw, last_frame);

      if (job_msg->job_number == std::numeric_limits<uint32_t>::max()) {
        save_images(pixels_raw, width, height, frame_number, true, true, job_msg->output_file);
        if (job_msg->last_frame) {
          finished = true;
        }
      } else {
        save_images(pixels_raw, width, height, frame_number, true, true, job_msg->output_file);
        current_frame++;
      }
    } else {
      break;
    }
    buffered_frames.erase(pos);
  }
  if (finished && webserv) {
    webserv->stop();
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
    metrics_->register_thread(i, fmt::format("L{}", i));
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
    webserv->set_script(script_);
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

const data::viewpoint &starcry::get_viewpoint() const {
  return viewpoint;
}

void starcry::set_viewpoint(data::viewpoint &vp) {
  viewpoint = vp;
}

// This function is used to send to framer and als the preview window
std::vector<uint32_t> starcry::pixels_vec_to_pixel_data(const std::vector<data::color> &pixels_in,
                                                        const data::settings &settings) {
  std::vector<uint32_t> pixels_out;
  pixels_out.reserve(pixels_in.size());

  if (settings.dithering) {
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

void starcry::pixels_vec_insert_checkers_background(std::vector<uint32_t> &pixels, int width, int height) {
  uint8_t *p = (uint8_t *)&pixels[0];
  int x = 0, y = 0;
  bool flag = true;
  for (size_t i = 0; i < (width * height); i++) {
    uint8_t &r = *(p++);
    uint8_t &g = *(p++);
    uint8_t &b = *(p++);
    uint8_t &a = *(p++);

    data::color checker;
    checker.r = flag ? 0.75 : 1.;
    checker.g = flag ? 0.75 : 1.;
    checker.b = flag ? 0.75 : 1.;
    checker.a = 1.;

    data::color current;
    current.r = double(r) / 255.;
    current.g = double(g) / 255.;
    current.b = double(b) / 255.;
    current.a = double(a) / 255.;

    data::color new_col = blend(checker, current);

    r = new_col.r * 255;
    g = new_col.g * 255;
    b = new_col.b * 255;
    a = new_col.a * 255;

    x++;
    if (x == width) {
      x = 0;
      y++;
      if (y % 20 == 0) {
        flag = !flag;
      }
    }
    if (x % 20 == 0) {
      flag = !flag;
    }
  }
}

void starcry::save_images(std::vector<data::color> &pixels_raw,
                          size_t width,
                          size_t height,
                          size_t frame_number,
                          bool write_8bit_png,
                          bool write_32bit_exr,
                          const std::string &output_file) {
  auto filename = fs::path(gen->filename()).stem().string();
  if (!pixels_raw.empty()) {
    // There is 16 BIT, also + Alpha, however, seems to internally still use an 8 bit palette somehow.
    // Will need to figure out in the future how to properly use 16 bit, for now, will focus on fixing the 8 bit
    // version. png::image<png::rgb_pixel_16> image(job.width, job.height);
    if (write_8bit_png) {
      png::image<png::rgba_pixel> image(width, height);
      copy_to_png(pixels_raw, width, height, image, gen->settings().dithering);
      if (output_file.size()) {
        image.write(fmt::format("{}.png", output_file));
      } else {
        image.write(fmt::format(
            "output_frame_{:05d}_seed_{}_{}x{}-{}.png", frame_number, gen->get_seed(), width, height, filename));
      }
    }

    // Save TIFF through ImageMagick
    // using namespace Magick;
    // try {
    //   std::vector<double> rgb;
    //   rgb.resize(width * height * 4);
    //   size_t index = 0;
    //   auto &source = pixels_raw;
    //   for (uint32_t y = 0; y < height; y++) {
    //     for (uint32_t x = 0; x < width; x++) {
    //       rgb.push_back(source[index].r);
    //       rgb.push_back(source[index].g);
    //       rgb.push_back(source[index].b);
    //       rgb.push_back(source[index].a);
    //       index++;
    //     }
    //   }
    //   Image image(fmt::format("{}x{}", width, height).c_str(), "white");
    //   image.read(width, height, "RGBA", StorageType::DoublePixel, (void *)&rgb[0]);
    //   image.write(
    //       fmt::format("output_frame_{}_seed_{}_{}x{}-{}.tiff", frame_number, gen->get_seed(), width, height,
    //       filename));
    // } catch (std::exception &error_) {
    //   std::cout << "Caught exception: " << error_.what() << std::endl;
    // }

    if (write_32bit_exr) {
      // Save EXR through OpenEXR directly
      using namespace Imf;
      int w = width;
      int h = height;

      Array2D<float> rp(h, w);
      Array2D<float> gp(h, w);
      Array2D<float> bp(h, w);
      Array2D<float> zp(h, w);

      auto &source = pixels_raw;
      size_t index = 0;
      for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
          rp[y][x] = source[index].r;
          gp[y][x] = source[index].g;
          bp[y][x] = source[index].b;
          zp[y][x] = source[index].a;
          index++;
        }
      }
      const float *rPixels = &rp[0][0];
      const float *gPixels = &gp[0][0];
      const float *bPixels = &bp[0][0];
      const float *zPixels = &zp[0][0];

      Header header(w, h);
      header.channels().insert("R", Channel(Imf::FLOAT));
      header.channels().insert("G", Channel(Imf::FLOAT));
      header.channels().insert("B", Channel(Imf::FLOAT));
      header.channels().insert("Z", Channel(Imf::FLOAT));

      std::string exr_filename;
      if (output_file.size()) {
        exr_filename = fmt::format("{}.exr", output_file);
      } else {
        exr_filename = fmt::format(
            "output_frame_{:05d}_seed_{}_{}x{}-{}.exr", frame_number, gen->get_seed(), width, height, filename);
      }
      OutputFile file(exr_filename.c_str(), header);
      FrameBuffer frameBuffer;
      frameBuffer.insert("R",                           // name
                         Slice(Imf::FLOAT,              // type
                               (char *)rPixels,         // base
                               sizeof(*rPixels) * 1,    // xStride
                               sizeof(*rPixels) * w));  // yStride
      frameBuffer.insert("G",                           // name
                         Slice(Imf::FLOAT,              // type
                               (char *)gPixels,         // base
                               sizeof(*gPixels) * 1,    // xStride
                               sizeof(*gPixels) * w));  // yStride
      frameBuffer.insert("B",                           // name
                         Slice(Imf::FLOAT,              // type
                               (char *)bPixels,         // base
                               sizeof(*bPixels) * 1,    // xStride
                               sizeof(*bPixels) * w));  // yStride

      frameBuffer.insert("Z",                           // name
                         Slice(Imf::FLOAT,              // type
                               (char *)zPixels,         // base
                               sizeof(*zPixels) * 1,    // xStride
                               sizeof(*zPixels) * w));  // yStride

      file.setFrameBuffer(frameBuffer);
      file.writePixels(h);
    }
  }
}
