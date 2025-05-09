/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry.h"

#include <algorithm>
#include <cstring>
#include <sstream>

#include <fmt/core.h>

#include "cereal/archives/json.hpp"
#include "zpp_bits.h"

#include "bitmap_wrapper.hpp"
#include "data/frame_request.hpp"
#include "data/reload_request.hpp"
#include "data/video_request.hpp"
#include "framer.hpp"
#include "generator.h"
#include "rendering_engine.h"
#include "util/image_splitter.hpp"
#include "util/image_utils.h"
#include "util/logger.h"
#include "webserver.h"

#include "interpreter/job_mapper.h"
#include "starcry/metrics.h"
#include "starcry/redis_client.h"
#include "starcry/redis_server.h"
#include "util/benchmark.h"
#include "util/scope_exit.hpp"
#include "util/threadname.hpp"

// TODO: re-organize this somehow
#include <sys/prctl.h>

#include <inotify-cpp/FileSystemAdapter.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-builtins"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <inotify-cpp/NotifierBuilder.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "gui_window.h"  // make sure to include after SeaSocks, BadRequest macro conflicts

starcry::starcry(starcry_options& options,
                 std::shared_ptr<v8_wrapper> context,
                 generator_state& state,
                 generator_config& config,
                 std::shared_ptr<Benchmark> benchmark)
    : context(context),
      options_(options),
      gen(nullptr),
      engines({}),
      system(std::make_shared<pipeline_system>(false)),
      cmds(system->create_queue("commands", options.concurrent_commands)),
      jobs(system->create_queue("jobs", options.concurrent_jobs)),
      frames(system->create_queue("frames", options.concurrent_frames)),
      pe(std::chrono::milliseconds(1000)),
      metrics_(std::make_shared<metrics>(options.notty || options.stdout_,
                                         [this]() {
                                           if (!gui)
                                             gui = std::make_unique<gui_window>();
                                           else
                                             gui->toggle_window();
                                         })),
      script_(options.script_file),
      notifier(nullptr),
      state_(state),
      config_(config),
      benchmark_(benchmark) {
  if (options.stdout_) {
    _stdout = true;
  }
  metrics_->set_script(script_);
  metrics_->init();
  set_metrics(&*metrics_);

  configure_inotify();
}

starcry::~starcry() {
  notifier->stop();
  metrics_->notify();
  pe.cancel();
  if (notifier_thread.joinable()) notifier_thread.join();
}

void starcry::configure_inotify() {
  if (!std::filesystem::exists("input")) {
    logger(DEBUG) << "path (input) does not exist" << std::endl;
    return;
  }
  inotifypp::filesystem::path path("input");
  auto handleNotification = [&](inotify::Notification notification) {
    logger(DEBUG) << "File modified on disk: " << notification.path.string() << std::endl;
    logger(DEBUG) << "File of interest: " << script_ << std::endl;
    if (notification.path.string() == script_) {
      // TODO: for the future implement HOT swapping (requires parsing JSON and merging intelligently)
      // TODO: fix this feature, currently it's super annoying, as it generates crashes
      auto req = std::make_shared<data::reload_request>(script_);
      add_reload_command(req);
    }
  };
  auto handleUnexpectedNotification = [](inotify::Notification notification) {};
  auto events = {inotify::Event::close_write};
  notifier = std::make_unique<inotify::NotifierBuilder>();
  if (!std::filesystem::exists(path)) {
    std::cout << "path (" << path << ") does not exist" << std::endl;
    return;
  }
  notifier->watchPathRecursively(path)
      .onEvents(events, handleNotification)
      .onUnexpectedEvent(handleUnexpectedNotification);
  notifier_thread = std::thread([&]() {
    set_thread_name("notifier");
    notifier->run();
  });
}

feature_settings& starcry::features() {
  return features_;
}

starcry_options& starcry::options() {
  return options_;
}

const std::string& starcry::script() {
  return script_;
}

void starcry::set_script(const std::string& script) {
  script_ = script;
  if (metrics_) metrics_->set_script(script_);
  if (webserv) webserv->set_script(script_);
}

void starcry::update_script_contents(const std::string& contents) {
  std::ofstream out(script_);
  out << contents;
  out.close();
}

void starcry::add_reload_command(std::shared_ptr<data::reload_request> req) {
  cmds->push(std::make_shared<reload_instruction>(req));
  if (webserv) {
    std::ifstream stream(req->script());
    if (!stream) {
      throw std::runtime_error("could not locate file " + req->script());
    }
    std::istreambuf_iterator<char> begin(stream), end;
    // remove "_ =" part from script (which is a workaround to silence 'not in use' linting)
    if (*begin == '_') {
      while (*begin != '=') begin++;
      begin++;
    }
    const auto source = std::string(begin, end);

    json j{
        {"type", "fs_change"},
        {"file", req->script()},
        {"source", source},
    };
    webserv->send_fs_change(j.dump());
  }
}

void starcry::add_image_command(std::shared_ptr<data::frame_request> req) {
  cmds->push(std::make_shared<frame_instruction>(req));
  pe.run([=, this]() {
    set_thread_name("periodic_exec");
    if (webserv) {
      webserv->send_stats(system->get_stats());
      webserv->send_metrics(metrics_->to_json());
    }
  });
}

void starcry::add_video_command(std::shared_ptr<data::video_request> req) {
  cmds->push(std::make_shared<video_instruction>(req));
}

image starcry::render_job(size_t thread_num,
                          rendering_engine& engine,
                          const data::job& job,
                          const data::settings& settings,
                          const std::vector<int64_t>& selected_ids) {
  prctl(PR_SET_NAME, fmt::format("sc {} {}/{}", job.frame_number, job.chunk, job.num_chunks).c_str(), NULL, NULL, NULL);

  // TODO: also not going to work well with multithreaded!! you need to just collect durations separately somehow
  const auto start = benchmark_ ? benchmark_->measure("frame rendering") : std::chrono::high_resolution_clock::now();
  render_params params{thread_num,
                       job.job_number == std::numeric_limits<uint32_t>::max() ? job.frame_number : job.job_number,
                       job.chunk,
                       job.num_chunks,
                       metrics_,
                       job.background_color,
                       job.shapes,
                       job.view_x,
                       job.view_y,
                       job.canvas_w,
                       job.canvas_h,
                       job.scale,
                       job.scales,
                       options_.level == log_level::debug,
                       settings,
                       options_.debug || get_viewpoint().debug,
                       selected_ids};

  scope_exit se([this, start]() {
    if (benchmark_) benchmark_->store("frame rendering", start);
  });

  return engine.render(params, job.offset_x, job.offset_y, job.width, job.height);
}

// MARK1 transform instruction into job (using generator)
void starcry::command_to_jobs(std::shared_ptr<instruction> cmd_def) {
  const auto start = benchmark_ ? benchmark_->measure("command_to_jobs") : std::chrono::high_resolution_clock::now();
  scope_exit se([this, start]() {
    if (benchmark_) benchmark_->store("command_to_jobs", start);
  });
  if (!gen) {
    context->recreate_isolate_in_this_thread();
    gen = interpreter::generator::create(metrics_, context, options().generator_opts, state_, config_, benchmark_);
  }

  if (const auto& instruction = std::dynamic_pointer_cast<reload_instruction>(cmd_def)) {
    gen->init(
        instruction->req().script(), options_.output_file, options_.rand_seed, options_.preview, features_.caching);
  }

  if (const auto& instruction = std::dynamic_pointer_cast<video_instruction>(cmd_def)) {
    auto& v = instruction->video_ref();
    if (viewpoint.canvas_w && viewpoint.canvas_h) {
      gen->init(v.script(),
                v.output_file(),
                options_.rand_seed,
                v.preview(),
                features_.caching,
                instruction->video().width(),
                instruction->video().height(),
                viewpoint.scale);
    } else {
      gen->init(v.script(), v.output_file(), options_.rand_seed, v.preview(), features_.caching);
    }

    // Update current_frame if we fast-forward to a different offset frame.
    if (v.offset_frames() > 0) {
      current_frame = v.offset_frames();
    }
    double use_fps = config_.fps;
    if (!framer && options().output && instruction->video().output_file() != "/dev/null") {
      auto stream_mode = frame_streamer::stream_mode::FILE;
      auto output_file = v.output_file();
      if (output_file.size() >= 4 && output_file.substr(output_file.size() - 4, 4) == "m3u8") {
        use_fps = 1000;
        stream_mode = frame_streamer::stream_mode::HLS;
      }
      if (v.output_file().empty()) {
        auto scriptname = fs::path(script_).stem().string();
        v.set_output_file(fmt::format(
            "output/{}-seed_{}_{}x{}.h264", scriptname, state_.seed, (int)state_.canvas_w, (int)state_.canvas_h));
      }
      if (options().video) {
        framer = std::make_unique<frame_streamer>(v.output_file(), stream_mode);
        framer->set_num_threads(options().num_ffmpeg_threads);
        framer->set_log_callback([&](int level, const std::string& line) {
          metrics_->log_callback(level, line);
        });
      }
    }
    size_t bitrate = (500 * 1024 * 8);  // TODO: make configurable
    if (framer) {
      // use canvas dimensions from state or otherwise from instruction
      // state_.canvas_w or instruction->video().width()
      // TODO: more side effects after boost::di that need investigation
      const auto use_width = state_.canvas_w ? state_.canvas_w : instruction->video().width();
      const auto use_height = state_.canvas_h ? state_.canvas_h : instruction->video().height();
      framer->initialize(bitrate, use_width, use_height, use_fps);
    }
    while (true) {
      auto ret = gen->generate_frame();
      auto job_copy = std::make_shared<data::job>(*gen->get_job());
      if (job_copy->frame_number < v.offset_frames()) {
        metrics_->skip_job(job_copy->job_number);
        continue;
      }
      // TODO: duplicated code
      if (v.num_chunks() == 1) {
        jobs->push(std::make_shared<job_message>(cmd_def, job_copy));
      } else {
        metrics_->resize_job(job_copy->job_number, v.num_chunks());
        util::ImageSplitter<uint32_t> is{job_copy->canvas_w, job_copy->canvas_h};
        const auto rectangles = is.split(v.num_chunks(), util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
        interpreter::job_mapper jm(job_copy);
        for (size_t i = 0, counter = 1; i < rectangles.size(); i++) {
          jm.map_rectangle(rectangles[i], counter, v.num_chunks());
          counter++;
          jobs->push(std::make_shared<job_message>(cmd_def, std::make_shared<data::job>(*job_copy)));
        }
      }
      jobs->sleep_until_not_full();
      if (!ret) break;
    }
    std::cout << std::endl;
    if (v.client() != nullptr) {
      return;  // prevent termination of queues
    }
  } else if (const auto& instruction = std::dynamic_pointer_cast<frame_instruction>(cmd_def)) {
    metrics_->clear();

    const auto& f = instruction->frame();

    try {
      if (viewpoint.canvas_w && viewpoint.canvas_h) {
        gen->init(f.script(),
                  f.output_file(),
                  options_.rand_seed,
                  f.preview(),
                  features_.caching,
                  viewpoint.canvas_w,
                  viewpoint.canvas_h,
                  viewpoint.scale);
        gen->set_checkpoints(checkpoints_);
      } else {
        gen->init(f.script(), f.output_file(), options_.rand_seed, f.preview(), features_.caching);
      }
    } catch (std::runtime_error& err) {
      logger(DEBUG) << "err = " << err.what() << std::endl;
      return;
    }
    gen->reset_seeds();
    gen->fast_forward(f.frame_num());
    gen->generate_frame();

    auto the_job = gen->get_job();
    interpreter::job_mapper jm(the_job);
    jm.map_viewpoint(viewpoint);
    jm.map_output_file(f.output_file());
    the_job->timeout = f.timeout();

    if (f.num_chunks() == 1) {
      jm.map_last_frame(f.last_frame());
      jobs->push(std::make_shared<job_message>(cmd_def, the_job));
    } else {
      metrics_->resize_job(the_job->job_number, f.num_chunks());
      util::ImageSplitter<uint32_t> is{the_job->canvas_w, the_job->canvas_h};
      const auto rectangles = is.split(f.num_chunks(), util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
      for (size_t i = 0, counter = 1; i < rectangles.size(); i++) {
        jm.map_rectangle(rectangles[i], counter, f.num_chunks());
        counter++;
        jm.map_last_frame(f.last_frame());
        jobs->push(std::make_shared<job_message>(cmd_def, std::make_shared<data::job>(*the_job)));
      }
    }
    if (f.client() != nullptr) {
      return;  // prevent termination of queues
    }
  } else if (const auto& instruction = std::dynamic_pointer_cast<reload_instruction>(cmd_def)) {
    logger(DEBUG) << "Reloaded." << std::endl;
    return;  // prevent termination of queues
  } else {
    logger(WARNING) << "No video or frame instruction provided." << std::endl;
  }
  logger(DEBUG) << "check for termination" << std::endl;
  cmds->check_terminate();
  jobs->check_terminate();
}

// MARK1 render job using renderer and transform into render_msg
std::shared_ptr<render_msg> starcry::job_to_frame(size_t i, std::shared_ptr<job_message> job_msg) {
  const auto start = benchmark_ ? benchmark_->measure("job_to_frame") : std::chrono::high_resolution_clock::now();
  scope_exit se([this, start]() {
    if (benchmark_) benchmark_->store("job_to_frame", start);
  });
  auto& job = *job_msg->job;

  if (!options_.render) {
    // Be careful, below is a stub message, note the zero width and height below, since we don't
    // intend to actually do rendering when we set the render flag to false. This is for testing
    // for example Javascript exclusively performance
    std::vector<uint32_t> transfer_pixels;
    auto msg = std::make_shared<render_msg>(job_msg, transfer_pixels);
    msg->set_height(0);
    msg->set_width(0);
    return msg;
  }

  // render
  data::settings settings = gen->settings();
  if (job.job_number == std::numeric_limits<uint32_t>::max()) {
    metrics_->set_frame_mode();
    metrics_->render_job(i, job.frame_number, job.chunk);
  } else {
    metrics_->render_job(i, job.job_number, job.chunk);
  }

  const auto selected_ids_transitive = ([&, this]() {
    std::vector<int64_t> ret;
    if (job_msg && job_msg->original_instruction) ret = this->selected_ids_transitive(job_msg);
    return ret;
  })();

  image bmp = render_job(i, *engines[i], job, settings, selected_ids_transitive);

  if (job.job_number == std::numeric_limits<uint32_t>::max()) {
    metrics_->complete_render_job(i, job.frame_number, job.chunk);
  } else {
    metrics_->complete_render_job(i, job.job_number, job.chunk);
  }

  // handle videos
  if (const auto& instruction = std::dynamic_pointer_cast<video_instruction>(job_msg->original_instruction)) {
    const auto& v = instruction->video();
    auto transfer_pixels = pixels_vec_to_pixel_data(bmp.pixels(), gen->settings().dithering);

    auto msg = std::make_shared<render_msg>(job_msg);
    msg->set_pixels(transfer_pixels);
    if (options().video) {
      if (v.raw_video()) {
        msg->set_raw(bmp.pixels());
      }
      // TODO: make this configurable for benchmarking
      msg->suspend();
      return msg;
    } else {
      if (v.raw_video()) {
        save_images(config_.filename,
                    rand_,
                    state_.seed,
                    gen->settings().dithering,
                    bmp.pixels(),
                    msg->width,
                    msg->height,
                    msg->original_job_message->job->frame_number,
                    false /* true */,
                    true,
                    job.output_file);
      }
      // interrupt the chain, pass along empty msg.
      std::vector<uint32_t> transfer_pixels;
      auto msg = std::make_shared<render_msg>(job_msg, transfer_pixels);
      msg->set_height(0);
      msg->set_width(0);
      return msg;
    }
  }

  // handle frames
  if (const auto& instruction = std::dynamic_pointer_cast<frame_instruction>(job_msg->original_instruction)) {
    const auto& f = instruction->frame();
    auto msg = std::make_shared<render_msg>(job_msg);
    job.job_number = std::numeric_limits<uint32_t>::max();

    if (f.raw_image() || get_viewpoint().raw || get_viewpoint().save) {
      // NOTE that currently bmp is kind of 'moved' into the msg..
      msg->set_raw(bmp.pixels());
    }

    if (f.raw_bitmap()) {
      auto transfer_pixels = pixels_vec_to_pixel_data(bmp.pixels(), gen->settings().dithering);
      msg->set_pixels(transfer_pixels);
    }

    if (instruction->frame_ptr() && !instruction->frame_ptr()->client() && f.compressed_image()) {
      png::image<png::rgba_pixel> image(job.width, job.height);
      copy_to_png(rand_, msg->pixels_raw, job.width, job.height, image, gen->settings().dithering);
      std::ostringstream ss;
      image.write_stream(ss);
      auto img = ss.str();
      msg->set_buffer(img);
    }

    if (f.renderable_shapes()) {
      std::ostringstream os;
      {
        cereal::JSONOutputArchive archive(os);
        archive(job);
      }
      auto str = os.str();
      msg->set_buffer(str);
    }

    if (f.metadata_objects() || get_viewpoint().labels) {
      auto buf = serialize_shapes_to_json(job_msg->job->shapes);
      msg->set_buffer(buf);
    }

    msg->set_width(get_viewpoint().canvas_w ? get_viewpoint().canvas_w : job.width);
    msg->set_height(get_viewpoint().canvas_h ? get_viewpoint().canvas_h : job.height);

    // TODO: below belongs somewhere else
    features().caching = get_viewpoint().caching;
    return msg;
  } else {
    throw std::runtime_error("expected a frame instruction");
  }
}

// MARK1 handle the render msg, create into video or whatever
void starcry::handle_frame(std::shared_ptr<render_msg> job_msg) {
  const auto start = benchmark_ ? benchmark_->measure("handle_frame") : std::chrono::high_resolution_clock::now();
  scope_exit se([this, start]() {
    if (benchmark_) benchmark_->store("handle_frame", start);
  });
  auto instr = job_msg->original_job_message->original_instruction;
  std::shared_ptr<data::frame_request> f = nullptr;
  std::shared_ptr<data::video_request> v = nullptr;
  if (const auto& instruction = std::dynamic_pointer_cast<video_instruction>(instr)) {
    v = instruction->video_ptr();
  } else if (const auto& instruction = std::dynamic_pointer_cast<frame_instruction>(instr)) {
    f = instruction->frame_ptr();
  }
  // auto type = job_msg->original_job_message->original_instruction->type2;
  bool finished = false;
  auto& job = *job_msg->original_job_message->job;  // this will allocate a lot of memory if copied
  auto job_client = f ? f->client() : v->client();
  if (options_.level != log_level::silent) {
    const auto frame = job.job_number == std::numeric_limits<size_t>::max() ? 0 : job.job_number;
    static size_t prev_frame = 0;
    if (options_.level != log_level::info) {
      if (frame != prev_frame) {
      }
    }
    prev_frame = frame;
  }

  if (!options_.render) {
    if (job.last_frame)
      if (webserv) webserv->stop();
    return;  // early exit
  }
  // COZ_PROGRESS;
  auto process = [&](size_t width,
                     size_t height,
                     std::vector<uint32_t>& pixels,
                     std::vector<data::color>& pixels_raw,
                     bool last_frame) {
    const auto add_frame_to_streamer = [&]() {
      if (gui) {
        gui->add_frame(width, height, pixels);
      }
      if (framer && pixels.size()) {
        framer->add_frame(pixels);
        if (last_frame) {
          for (int i = 0; i < 50; i++) {
            framer->add_frame(pixels);
          }
        }
      }
    };

    if (job_client == nullptr) {
      // get pixels from raw pixels for the ffmpeg video
      if (pixels.empty() && !pixels_raw.empty()) {
        std::vector<uint32_t> pixels_new = pixels_vec_to_pixel_data(pixels_raw, gen->settings().dithering);
        std::swap(pixels, pixels_new);
      }
      // insert checkers background
      if (pixels.size() < (width * height)) {
        std::cerr << "pixels.size(), " << pixels.size() << " < (" << width << " * " << height << ")" << std::endl;
      } else {
        pixels_vec_insert_checkers_background(pixels, width, height);
      }
      add_frame_to_streamer();
      return last_frame;
    } else {
      // Always use checkerboard for interactive mode.
      // TODO: this needs to double check if it can insert, not overflow
      pixels_vec_insert_checkers_background(pixels, width, height);
    }

    add_frame_to_streamer();

    std::swap(job_msg->pixels, pixels);
    if (options().interactive) {
      // do not swap, we're sending chunks to the browser
      pixels_raw = job_msg->pixels_raw;
    } else {
      std::swap(job_msg->pixels_raw, pixels_raw);
    }

    if (job_client != nullptr && f && (f->raw_bitmap() || f->raw_image())) {
      if (webserv && !options().interactive) {
        // This code is a little verbose, but somehow I had issues with job_client and other values such as job_client,
        // not being (properly?) captured. I'm now passing stuff explicitly as parameters. My guess is that for reasons
        // beyond my understanding I was hitting some undefined behavior issue. And the compiler was optimizing stuff,
        // that resulted in inconsistent memory. I'm happy after hours of debugging that this more verbose versions
        // seems to be doing the right thing consistently.
        webserv->execute_bitmap(
            [job_client, width, height](std::shared_ptr<BitmapHandler> bmp_handler,
                                        std::shared_ptr<render_msg> job_msg) {
              callback_to_bmp_handler(bmp_handler, job_msg, job_client, width, height, 1, 1);
            },
            job_msg);
      }
    }

    if (job_client != nullptr && f && f->metadata_objects()) {
      job_msg->ID = webserv->get_client_id(job_client);
      if (webserv) {
        webserv->execute_objects(
            [job_client, width, height](std::shared_ptr<ObjectsHandler> objects_handler,
                                        std::shared_ptr<render_msg> job_msg) {
              callback_to_objects_handler(objects_handler, job_msg, job_client, width, height);
            },
            job_msg);
      }
    }

    return finished;
  };

  buffered_frames[job.job_number].push_back(job_msg);

  if (f != nullptr) {
    if (f->renderable_shapes() || f->metadata_objects()) {
      current_frame = job.frame_number;
    }
  }

  bool flag = buffered_frames.size();
  while (flag) {
    const auto process_frame =
        [this, &process, &finished, &job](std::map<size_t, std::vector<std::shared_ptr<render_msg>>>::iterator pos) {
          // sort them first
          std::sort(pos->second.begin(), pos->second.end(), [](const auto& lh, const auto& rh) {
            return lh->original_job_message->job->chunk < rh->original_job_message->job->chunk;
          });

          // constructed full frame data (all chunks combined
          // TODO: combine this in a struct
          std::vector<uint32_t> pixels;
          std::vector<data::color> pixels_raw;
          size_t width = 0;
          size_t height = 0;
          bool last_frame = false;
          size_t frame_number = 0;

          size_t reserve_size =
              std::accumulate(pos->second.begin(), pos->second.end(), size_t(0), [](size_t size, auto& chunk) {
                return chunk->pixels.size() + size;
              });
          size_t reserve_size2 =
              std::accumulate(pos->second.begin(), pos->second.end(), size_t(0), [](size_t size, auto& chunk) {
                return chunk->pixels_raw.size() + size;
              });
          pixels.reserve(reserve_size);
          pixels_raw.reserve(reserve_size2);
          for (auto& chunk : pos->second) {
            chunk->unsuspend();
            pixels.insert(std::end(pixels), std::begin(chunk->pixels), std::end(chunk->pixels));
            pixels_raw.insert(std::end(pixels_raw), std::begin(chunk->pixels_raw), std::end(chunk->pixels_raw));
            chunk->pixels.clear();
            chunk->pixels.shrink_to_fit();
            // don't mutilate the chunks in interactive mode, we're sending them to the browser
            if (!options().interactive) {
              chunk->pixels_raw.clear();
              chunk->pixels_raw.shrink_to_fit();
            }

            // these don't have to be accumulated
            width = chunk->width;
            height = chunk->height;

            last_frame = chunk->original_job_message->job->last_frame;
            if (last_frame) finished = true;
            frame_number = chunk->original_job_message->job->frame_number;
          }

          finished = process(width, height, pixels, pixels_raw, last_frame);

          save_images(config_.filename,
                      rand_,
                      state_.seed,
                      gen->settings().dithering,
                      pixels_raw,
                      width,
                      height,
                      frame_number,
                      true,
                      true,
                      job.output_file);

          if (job.job_number == std::numeric_limits<uint32_t>::max()) {
            if (job.last_frame) {
              finished = true;
            }
          }
          buffered_frames.erase(pos);
        };

    const auto try_and_call = [this, &process_frame](size_t frame) -> bool {
      auto pos = buffered_frames.find(frame);
      if (pos != buffered_frames.end()) {
        if (pos->second.size() == pos->second[0]->original_job_message->job->num_chunks) {
          process_frame(pos);
          return true;
        }
      }
      return false;
    };

    // videos have to be explicit about each frame one by one in the correct order
    if (try_and_call(current_frame)) {
      current_frame++;
    }
    // individual pictures (-f) don't need to be, and can use this sentinel 'max' value.
    else if (!try_and_call(std::numeric_limits<uint32_t>::max())) {
      flag = false;
    }
  }
  if (webserv) {
    if (!options().interactive) {
      webserv->stop();
    } else if (f && job_client) {
      if (webserv) {
        const auto width = job_msg->original_job_message->job->width;
        const auto height = job_msg->original_job_message->job->height;
        const auto chunk = job_msg->original_job_message->job->chunk;
        const auto num_chunks = job_msg->original_job_message->job->num_chunks;
        webserv->execute_bitmap(
            [job_client, width, height, chunk, num_chunks](std::shared_ptr<BitmapHandler> bmp_handler,
                                                           std::shared_ptr<render_msg> job_msg) {
              callback_to_bmp_handler(bmp_handler, job_msg, job_client, width, height, chunk, num_chunks);
            },
            job_msg);
      }
    }
  }
}

void starcry::setup_server(const std::string& host) {
  if (options_.gui) gui = std::make_unique<gui_window>();

  system->spawn_consumer<instruction>(
      "generator", std::bind(&starcry::command_to_jobs, this, std::placeholders::_1), cmds);

  if (options_.enable_remote_workers) {
    redisserver = std::make_shared<redis_server>(host, *this /*jobs, frames*/);
    redisserver->run();
  }

  for (size_t i = 0; i < options_.num_worker_threads; i++) {
    const auto renderer_name = fmt::format("renderer-{}", i);
    metrics_->register_thread(i, fmt::format("L{}", i));
    engines[i] = std::make_shared<rendering_engine>();
    system->spawn_transformer<job_message>(renderer_name,
                                           std::bind(&starcry::job_to_frame, this, i, std::placeholders::_1),
                                           jobs,
                                           frames,
                                           transform_type::same_pool);
  }

  system->spawn_consumer<render_msg>(
      "streamer", std::bind(&starcry::handle_frame, this, std::placeholders::_1), frames);

  system->start(false);
}

void starcry::run() {
  if (options_.webserver) {
    webserv = std::make_shared<webserver>(this);
    webserv->set_script(script_);
    webserv->run();  // blocks
  }
  system->explicit_join();
  if (gui) gui->finalize();
  if (framer) framer->finalize();
  if (benchmark_) {
    benchmark_->writeResults();
  }
  std::cout << std::endl;
}

void starcry::run_client(const std::string& host) {
  redis_client redisclient(host, *this);
  rendering_engine engine;
  bitmap_wrapper bitmap;

  redisclient.run(bitmap, engine);
}

const data::viewpoint& starcry::get_viewpoint() const {
  return viewpoint;
}

void starcry::set_viewpoint(data::viewpoint& vp) {
  viewpoint = vp;
}
std::string __copy__shapes_binary_to_json(const std::string& binary) {
  std::cout << "shapes_binary_to_json begin: " << binary.size() << std::endl;
  std::istringstream is(binary);
  std::cout << "next = 1" << std::endl;
  cereal::BinaryInputArchive archive(is);
  std::cout << "next = 2" << std::endl;
  std::vector<data::shape> shapes;
  std::cout << "next = 3" << std::endl;
  try {
    std::cout << "next = 3A" << std::endl;
    archive(shapes);  // this fucker crashes in webassembly...
    std::cout << "next = 3B" << std::endl;
  } catch (std::exception& e) {
    std::cout << "exception during archive: " << e.what() << std::endl;
    throw e;
  } catch (...) {
    std::cout << "unexpected exception during archive" << std::endl;
    throw;
  }
  std::cout << "next = 4" << std::endl;
  std::cout << "shapes_binary_to_json archived" << std::endl;
  std::string json_output;
  if (!data::shapes_to_json(shapes, json_output)) {
    // logger(ERROR) << "Failed to convert shape to json" << std::endl;
    std::cout << "failed to convert" << std::endl;
  }
  std::cout << "the end ??" << std::endl;
  return json_output;
}

std::string starcry::serialize_shapes_to_json(std::vector<std::vector<data::shape>>& shapes) {
  json shapes_json = {};
  if (!shapes.empty()) {
#define DEBUG_NUM_SHAPES
#ifdef DEBUG_NUM_SHAPES
    std::unordered_map<int64_t, int64_t> nums;
    for (size_t i = 0; i < shapes.size(); i++) {
      for (const auto& shape : shapes[i]) {
        nums[shape.unique_id]++;
      }
    }
#endif
    std::string out;
    if (!data::shapes_to_json(shapes[shapes.size() - 1], out)) {
      logger(ERROR) << "Failed to convert shapes to JSON" << std::endl;
    }
    return out;
  }
  return shapes_json.dump();
}

std::vector<int64_t> starcry::selected_ids_transitive(std::shared_ptr<job_message>& job) {
  const std::vector<int64_t> selected_ids = ([&]() {
    {
      if (const auto& instruction = std::dynamic_pointer_cast<frame_instruction>(job->original_instruction)) {
        if (instruction->frame_ptr()) {
          return instruction->frame_ptr()->selected_ids();
        }
      }
    }
    return std::vector<int64_t>{};
  })();
  std::vector<int64_t> selected_ids_transitive;
  if (!selected_ids.empty()) {
    selected_ids_transitive = gen->get_transitive_ids(selected_ids);
  }
  return selected_ids_transitive;
}

void starcry::set_checkpoint(int frame) {
  checkpoints_.insert(frame);
}

std::string starcry::get_js_api() {
  if (!gen) {
    // this is rather expensive, but it's fine
    auto gen = interpreter::generator::create(metrics_, context, options().generator_opts, state_, config_, benchmark_);
    return gen->get_js_api();
  }
  return gen->get_js_api();
}

std::string starcry::get_spec(const std::string& component) {
  if (!gen) return "*** GENERATOR NOT INITIALIZED ***";
  return gen->get_spec(component);
}
