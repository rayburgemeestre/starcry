/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <data/job.hpp>
#include <data/pixels.hpp>
#include <memory>
#include <mutex>

#include "image.hpp"
#include "messages.hpp"
#include "piper.h"
#include "png.hpp"

#include "util/limited_executor.hpp"

class sfml_window;
class bitmap_wrapper;
class generator_v2;
class rendering_engine_wrapper;
class render_server;
class webserver;
class frame_streamer;
class render_client;

namespace data {
struct job;
}

namespace seasocks {
class WebSocket;
}

class starcry {
public:
  enum class render_video_mode { generate_only, render_only, video_only, video_with_gui, gui_only };

private:
  size_t num_local_engines;
  bool enable_remote_workers;
  bool is_interactive;
  bool start_webserver;
  bool enable_compression;

  std::map<int, std::shared_ptr<bitmap_wrapper>> bitmaps;
  std::shared_ptr<generator_v2> gen;
  std::map<int, std::shared_ptr<rendering_engine_wrapper>> engines;
  std::shared_ptr<pipeline_system> system;
  std::shared_ptr<queue> cmds;
  std::shared_ptr<queue> jobs;
  std::shared_ptr<queue> frames;
  std::shared_ptr<render_server> renderserver;
  std::shared_ptr<sfml_window> gui = nullptr;
  std::shared_ptr<frame_streamer> framer = nullptr;
  std::shared_ptr<webserver> webserv = nullptr;
  render_video_mode mode;
  int64_t num_queue_per_worker = 1;
  std::function<void(starcry &sc)> on_pipeline_initialized;

  std::map<size_t, std::shared_ptr<render_msg>> buffered_frames;
  size_t current_frame = 0;
  limited_executor le;

public:
  starcry(
      size_t num_local_engines,
      bool enable_remote_workers,
      bool visualization_enabled,
      bool is_interactive,
      bool start_webserver,
      bool enable_compression,
      render_video_mode mode,
      std::function<void(starcry &sc)> on_pipeline_initialized = [](auto &) {});
  ~starcry();

  void add_command(seasocks::WebSocket *client, const std::string &script, instruction_type it, int frame_num);
  void add_command(seasocks::WebSocket *client, const std::string &script, const std::string &output_file);

  void run_server();
  void run_client();

private:
  void render_job(rendering_engine_wrapper &engine, const data::job &job, image &bmp);
  void copy_to_png(const std::vector<data::color> &source,
                   uint32_t width,
                   uint32_t height,
                   png::image<png::rgb_pixel> &dest);

  void command_to_jobs(std::shared_ptr<instruction> cmd_def);
  std::shared_ptr<render_msg> job_to_frame(size_t i, std::shared_ptr<job_message> job_msg);
  bool on_client_message(int sockfd, int type, size_t len, const std::string &data);
  bool on_server_message(render_client &client,
                         rendering_engine_wrapper &engine,
                         bitmap_wrapper &bitmap,
                         int sockfd,
                         int type,
                         size_t len,
                         const std::string &data);
  void handle_frame(std::shared_ptr<render_msg> job_msg);

  std::vector<uint32_t> pixels_vec_to_pixel_data(const std::vector<data::color> &pixels_in) const;
};
