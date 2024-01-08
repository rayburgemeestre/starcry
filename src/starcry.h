/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include "image.hpp"
#include "messages.hpp"
#include "piper.h"

#include "data/job.hpp"
#include "data/pixels.hpp"
#include "data/viewpoint.hpp"

#include "core/periodic_executor.hpp"
#include "interpreter/generator_options.hpp"
#include "util/random.hpp"

class v8_wrapper;
class sfml_window;
class bitmap_wrapper;
class rendering_engine;
class webserver;
class frame_streamer;
class redis_server;

namespace interpreter {
class generator;
}

namespace data {
struct job;
struct settings;
class reload_request;
class frame_request;
class video_request;
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

enum class log_level { silent, info, debug };

struct starcry_options {
  std::string script_file = "input/test.js";
  std::string output_file;
  size_t frame_of_interest = std::numeric_limits<size_t>::max();
  size_t frame_offset = 0;

  bool webserver = false;
  bool interactive = false;
  bool preview = false;
  // new
  bool gui = false;
  bool render = true;
  bool output = false;
  bool notty = false;
  bool stdout_ = false;
  bool debug = false;

  size_t num_worker_threads = std::thread::hardware_concurrency();
  // large number of chunks can actually significantly reduce performance if the number of objects is large relative to
  // the render time required since all objects have to be copied for each chunk.
  size_t num_chunks = 1;

  std::string host = "localhost";
  bool enable_remote_workers = false;
  bool compression = false;

  std::optional<double> rand_seed;

  log_level level = log_level::info;

  int concurrent_commands = 10;
  int concurrent_jobs = 10;
  int concurrent_frames = 10;

  generator_options generator_opts;
};

class starcry {
  friend class redis_server;
  friend class redis_client;

private:
  std::shared_ptr<v8_wrapper> context;

  starcry_options options_;
  feature_settings features_;

  std::map<int, std::shared_ptr<bitmap_wrapper>> bitmaps;
  std::shared_ptr<interpreter::generator> gen;
  std::map<int, std::shared_ptr<rendering_engine>> engines;
  std::shared_ptr<pipeline_system> system;
  std::shared_ptr<queue> cmds;
  std::shared_ptr<queue> jobs;
  std::shared_ptr<queue> frames;
  std::shared_ptr<redis_server> redisserver;
  std::shared_ptr<sfml_window> gui = nullptr;
  std::shared_ptr<frame_streamer> framer = nullptr;
  std::shared_ptr<webserver> webserv = nullptr;
  int64_t num_queue_per_worker = 1;

  std::map<size_t, std::vector<std::shared_ptr<render_msg>>> buffered_frames;
  size_t current_frame = 1;
  periodic_executor pe;
  std::shared_ptr<metrics> metrics_;

  data::viewpoint viewpoint;

  std::string script_;
  std::unique_ptr<inotify::NotifierBuilder> notifier;
  std::thread notifier_thread;

  util::random_generator rand_;

public:
  starcry(starcry_options &options, std::shared_ptr<v8_wrapper> &context);
  ~starcry();

  starcry_options &options();
  feature_settings &features();

  const std::string &script();
  void set_script(const std::string &script);
  void update_script_contents(const std::string &contents);

  void add_reload_command(std::shared_ptr<data::reload_request> req);
  void add_image_command(std::shared_ptr<data::frame_request> req);
  void add_video_command(std::shared_ptr<data::video_request> req);

  void setup_server(const std::string &host = "");

  void run();
  void run_client(const std::string &host);

  const data::viewpoint &get_viewpoint() const;
  void set_viewpoint(data::viewpoint &vp);

private:
  void configure_inotify();

  void render_job(size_t thread_num,
                  rendering_engine &engine,
                  const data::job &job,
                  image &bmp,
                  const data::settings &settings,
                  const std::vector<int64_t> &selected_ids = {});

  void command_to_jobs(std::shared_ptr<instruction> cmd_def);
  std::shared_ptr<render_msg> job_to_frame(size_t i, std::shared_ptr<job_message> job_msg);
  void handle_frame(std::shared_ptr<render_msg> job_msg);
  static std::string serialize_shapes_to_json(std::vector<std::vector<data::shape>>& shapes);
};
