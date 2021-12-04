/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <mutex>

#include "image.hpp"
#include "messages.hpp"
#include "piper.h"

#include "data/job.hpp"
#include "data/pixels.hpp"
#include "data/viewpoint.hpp"

#include "util/periodic_executor.hpp"

class sfml_window;
class bitmap_wrapper;
class generator;
class rendering_engine_wrapper;
class render_server;
class webserver;
class frame_streamer;
class render_client;
class server_message_handler;
class client_message_handler;

namespace data {
struct job;
struct settings;
}  // namespace data

namespace seasocks {
class WebSocket;
}

class command_handler;
class metrics;

struct feature_settings {
  bool caching = false;
};

namespace inotify {
class NotifierBuilder;
}

class starcry {
  friend class command_handler;
  friend class command_get_video;
  friend class command_get_bitmap;
  friend class command_get_image;
  friend class command_get_shapes;
  friend class command_get_objects;
  friend class command_get_raw_image;
  friend class server_message_handler;
  friend class client_message_handler;

public:
  enum class render_video_mode { generate_only, render_only, video_only, video_with_gui, gui_only, javascript_only };
  enum class log_level { silent, info, debug };

private:
  size_t num_local_engines;
  bool enable_remote_workers;
  bool start_webserver;
  bool enable_compression;
  feature_settings features_;

  std::map<int, std::shared_ptr<bitmap_wrapper>> bitmaps;
  std::shared_ptr<generator> gen;
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

  std::map<size_t, std::vector<std::shared_ptr<render_msg>>> buffered_frames;
  size_t current_frame = 1;
  periodic_executor pe;
  std::optional<double> seed;
  std::map<instruction_type, std::shared_ptr<command_handler>> command_handlers;
  std::shared_ptr<server_message_handler> server_message_handler_;
  std::shared_ptr<client_message_handler> client_message_handler_;
  log_level log_level_;
  std::shared_ptr<metrics> metrics_;

  data::viewpoint viewpoint;

  std::string script_;
  std::unique_ptr<inotify::NotifierBuilder> notifier;
  std::thread notifier_thread;

public:
  starcry(
      size_t num_local_engines,
      bool enable_remote_workers,
      log_level level,
      bool notty,
      bool start_webserver,
      bool enable_compression,
      render_video_mode mode,
      std::function<void(starcry &sc)> on_pipeline_initialized = [](auto &) {},
      std::optional<double> rand_seed = std::nullopt);
  ~starcry();

  feature_settings &features();

  void set_script(const std::string &script);

  void add_command(seasocks::WebSocket *client,
                   const std::string &script,
                   instruction_type it,
                   int frame_num,
                   int num_chunks,
                   bool raw,
                   bool preview,
                   bool last_frame,
                   const std::string &output_filename);
  void add_command(seasocks::WebSocket *client,
                   const std::string &script,
                   const std::string &output_file,
                   int num_chunks,
                   bool raw,
                   bool preview,
                   size_t offset_frames);

  void run_server();
  void run_client(const std::string &host);

  const data::viewpoint &get_viewpoint() const;
  void set_viewpoint(data::viewpoint &vp);

private:
  void render_job(size_t thread_num,
                  rendering_engine_wrapper &engine,
                  const data::job &job,
                  image &bmp,
                  const data::settings &settings);

  void command_to_jobs(std::shared_ptr<instruction> cmd_def);
  std::shared_ptr<render_msg> job_to_frame(size_t i, std::shared_ptr<job_message> job_msg);
  void handle_frame(std::shared_ptr<render_msg> job_msg);

  static std::vector<uint32_t> pixels_vec_to_pixel_data(const std::vector<data::color> &pixels_in,
                                                        const data::settings &settings);
  void save_images(std::vector<data::color> &pixels_raw,
                   size_t width,
                   size_t height,
                   size_t frame_number,
                   bool write_8bit_png,
                   bool write_32bit_exr,
                   const std::string &output_file);
};
