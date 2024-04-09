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

#include "bitmap_wrapper.hpp"
#include "data/frame_request.hpp"
#include "data/reload_request.hpp"
#include "data/video_request.hpp"
#include "framer.hpp"
#include "generator.h"
#include "rendering_engine.h"
#include "sfml_window.h"
#include "util/image_splitter.hpp"
#include "util/image_utils.h"
#include "util/logger.h"
#include "webserver.h"

#include "starcry/metrics.h"
#include "starcry/redis_client.h"
#include "starcry/redis_server.h"
#include "util/threadname.hpp"

// TODO: re-organize this somehow
#include <sys/prctl.h>

#include <inotify-cpp/FileSystemAdapter.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-builtins"
#endif
#include <inotify-cpp/NotifierBuilder.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "nlohmann/json.hpp"
using json = nlohmann::json;

starcry::starcry(starcry_options& options, std::shared_ptr<v8_wrapper>& context)
    : context(context),
      options_(options),
      bitmaps({}),
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
                                             gui = std::make_unique<sfml_window>();
                                           else
                                             gui->toggle_window();
                                         })),
      script_(options.script_file),
      notifier(nullptr) {
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

void starcry::render_job(size_t thread_num,
                         rendering_engine& engine,
                         const data::job& job,
                         image& bmp,
                         const data::settings& settings,
                         const std::vector<int64_t>& selected_ids) {
  prctl(PR_SET_NAME, fmt::format("sc {} {}/{}", job.frame_number, job.chunk, job.num_chunks).c_str(), NULL, NULL, NULL);

  bmp = engine.render(thread_num,
                      job.job_number == std::numeric_limits<uint32_t>::max() ? job.frame_number : job.job_number,
                      job.chunk,
                      job.num_chunks,
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
                      options_.level == log_level::debug,
                      settings,
                      options_.debug || get_viewpoint().debug,
                      selected_ids);
}

// MARK1 transform instruction into job (using generator)
void starcry::command_to_jobs(std::shared_ptr<instruction> cmd_def) {
  if (!gen) {
    context->recreate_isolate_in_this_thread();
    gen = std::make_shared<interpreter::generator>(metrics_, context, options().generator_opts);
  }

  if (const auto& instruction = std::dynamic_pointer_cast<reload_instruction>(cmd_def)) {
    gen->init(instruction->req().script(), options_.rand_seed, options_.preview, features_.caching);
  }

  if (const auto& instruction = std::dynamic_pointer_cast<video_instruction>(cmd_def)) {
    auto& v = instruction->video_ref();
    if (viewpoint.canvas_w && viewpoint.canvas_h) {
      gen->init(v.script(),
                options_.rand_seed,
                v.preview(),
                features_.caching,
                instruction->video().width(),
                instruction->video().height(),
                viewpoint.scale);
    } else {
      gen->init(v.script(), options_.rand_seed, v.preview(), features_.caching);
    }

    // Update current_frame if we fast-forward to a different offset frame.
    if (v.offset_frames() > 0) {
      current_frame = v.offset_frames();
    }
    double use_fps = gen->fps();
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
            "output_seed_{}_{}x{}-{}.h264", gen->get_seed(), (int)gen->width(), (int)gen->height(), scriptname));
      }
      framer = std::make_unique<frame_streamer>(v.output_file(), stream_mode);
      framer->set_num_threads(options().num_ffmpeg_threads);
      framer->set_log_callback([&](int level, const std::string& line) {
        metrics_->log_callback(level, line);
      });
    }
    size_t bitrate = (500 * 1024 * 8);  // TODO: make configurable
    if (framer) framer->initialize(bitrate, gen->width(), gen->height(), use_fps);
    while (true) {
      auto ret = gen->generate_frame();
      auto job_copy = std::make_shared<data::job>(*gen->get_job());
      if (job_copy->frame_number < v.offset_frames()) {
        metrics_->skip_job(job_copy->job_number);
        continue;
      }
      // TODO: duplicated code
      util::ImageSplitter<uint32_t> is{job_copy->canvas_w, job_copy->canvas_h};
      if (v.num_chunks() == 1) {
        jobs->push(std::make_shared<job_message>(cmd_def, job_copy));
      } else {
        metrics_->resize_job(job_copy->job_number, v.num_chunks());
        const auto rectangles = is.split(v.num_chunks(), util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
        for (size_t i = 0, counter = 1; i < rectangles.size(); i++) {
          job_copy->width = rectangles[i].width();
          job_copy->height = rectangles[i].height();
          job_copy->offset_x = rectangles[i].x();
          job_copy->offset_y = rectangles[i].y();
          job_copy->chunk = counter;
          job_copy->num_chunks = v.num_chunks();
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
                  options_.rand_seed,
                  f.preview(),
                  features_.caching,
                  viewpoint.canvas_w,
                  viewpoint.canvas_h,
                  viewpoint.scale);
        gen->set_checkpoints(checkpoints_);
      } else {
        gen->init(f.script(), options_.rand_seed, f.preview(), features_.caching);
      }
    } catch (std::runtime_error& err) {
      logger(DEBUG) << "err = " << err.what() << std::endl;
      return;
    }
    gen->reset_seeds();
    gen->fast_forward(f.frame_num());
    gen->generate_frame();

    auto the_job = gen->get_job();

    if (viewpoint.canvas_w) {
      the_job->canvas_w = viewpoint.canvas_w;
      the_job->width = viewpoint.canvas_w;
    }
    if (viewpoint.canvas_h) {
      the_job->canvas_h = viewpoint.canvas_h;
      the_job->height = viewpoint.canvas_h;
    }

    util::ImageSplitter<uint32_t> is{the_job->canvas_w, the_job->canvas_h};

    the_job->scale *= viewpoint.scale;
    the_job->view_x = viewpoint.offset_x;
    the_job->view_y = viewpoint.offset_y;
    the_job->output_file = f.output_file();

    if (f.num_chunks() == 1) {
      the_job->last_frame = f.last_frame();
      jobs->push(std::make_shared<job_message>(cmd_def, the_job));
    } else {
      metrics_->resize_job(the_job->job_number, f.num_chunks());
      const auto rectangles = is.split(f.num_chunks(), util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
      for (size_t i = 0, counter = 1; i < rectangles.size(); i++) {
        the_job->width = rectangles[i].width();
        the_job->height = rectangles[i].height();
        the_job->offset_x = rectangles[i].x();
        the_job->offset_y = rectangles[i].y();
        the_job->chunk = counter;
        the_job->num_chunks = f.num_chunks();
        counter++;

        the_job->job_number = std::numeric_limits<uint32_t>::max();
        the_job->last_frame = f.last_frame();
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
  auto& bmp = bitmaps[i]->get(job.width, job.height);
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

  render_job(i, *engines[i], job, bmp, settings, selected_ids_transitive);

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
    if (v.raw_video()) {
      msg->set_raw(bmp.pixels());
    }
    return msg;
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
      if (framer) {
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
      pixels_vec_insert_checkers_background(pixels, width, height);
    }

    add_frame_to_streamer();

    std::swap(job_msg->pixels, pixels);
    std::swap(job_msg->pixels_raw, pixels_raw);

    if (job_client != nullptr && f && (f->raw_bitmap() || f->raw_image())) {
      auto fun = [&](std::shared_ptr<BitmapHandler> bmp_handler,
                     std::shared_ptr<render_msg> job_msg,
                     seasocks::WebSocket* job_client,
                     uint32_t width,
                     uint32_t height) {
        std::string buffer;
        if (job_msg->pixels.size())
          for (const auto& i : job_msg->pixels) {
            buffer.append((char*)&i, sizeof(i));
          }
        else
          for (const auto& i : job_msg->pixels_raw) {
            uint32_t pixel = 0;
            pixel |= (int(i.a * 255) << 24);
            pixel |= (int(i.b * 255) << 16);
            pixel |= (int(i.g * 255) << 8);
            pixel |= (int(i.r * 255) << 0);
            buffer.append((char*)&pixel, sizeof(pixel));
          }
        bmp_handler->callback(job_client, buffer, width, height);
      };
      if (webserv) {
        // This code is a little verbose, but somehow I had issues with job_client and other values such as job_client,
        // not being (properly?) captured. I'm now passing stuff explicitly as parameters. My guess is that for reasons
        // beyond my understanding I was hitting some undefined behavior issue. And the compiler was optimizing stuff,
        // that resulted in inconsistent memory. I'm happy after hours of debugging that this more verbose versions
        // seems to be doing the right thing consistently.
        webserv->execute_bitmap(
            [&fun, job_client, width, height](std::shared_ptr<BitmapHandler> bmp_handler,
                                              std::shared_ptr<render_msg> job_msg) {
              fun(bmp_handler, job_msg, job_client, width, height);
            },
            job_msg);
      }
    }

    if (job_client != nullptr && f && f->compressed_image()) {
      auto fun = [&](std::shared_ptr<ImageHandler> image_handler,
                     std::shared_ptr<render_msg> job_msg,
                     seasocks::WebSocket* job_client,
                     uint32_t width,
                     uint32_t height) {
        image_handler->callback(job_client, job_msg->buffer);
      };
      if (webserv) {
        webserv->execute_image(
            [&fun, job_client, width, height](std::shared_ptr<ImageHandler> image_handler,
                                              std::shared_ptr<render_msg> job_msg) {
              fun(image_handler, job_msg, job_client, width, height);
            },
            job_msg);
      }
    }

    if (job_client != nullptr && f && f->metadata_objects()) {
      job_msg->ID = webserv->get_client_id(job_client);
      auto fun = [&](std::shared_ptr<ObjectsHandler> objects_handler,
                     std::shared_ptr<render_msg> job_msg,
                     seasocks::WebSocket* job_client,
                     uint32_t width,
                     uint32_t height) {
        if (objects_handler->_links.contains(job_msg->ID)) {
          auto con = objects_handler->_links[job_msg->ID];  // find con that matches ID this msg is from
          objects_handler->callback(con, job_msg->buffer);
        }
      };
      if (webserv) {
        webserv->execute_objects(
            [&fun, job_client, width, height](std::shared_ptr<ObjectsHandler> objects_handler,
                                              std::shared_ptr<render_msg> job_msg) {
              fun(objects_handler, job_msg, job_client, width, height);
            },
            job_msg);
      }
    }

    if (job_client != nullptr && f && f->renderable_shapes()) {
      auto fun = [&](std::shared_ptr<ShapesHandler> shapes_handler,
                     std::shared_ptr<render_msg> job_msg,
                     seasocks::WebSocket* job_client,
                     uint32_t width,
                     uint32_t height) {
        shapes_handler->callback(job_client, job_msg->buffer);
      };
      if (webserv) {
        webserv->execute_shapes(
            [&fun, job_client, width, height](std::shared_ptr<ShapesHandler> shapes_handler,
                                              std::shared_ptr<render_msg> job_msg) {
              fun(shapes_handler, job_msg, job_client, width, height);
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
            pixels.insert(std::end(pixels), std::begin(chunk->pixels), std::end(chunk->pixels));
            pixels_raw.insert(std::end(pixels_raw), std::begin(chunk->pixels_raw), std::end(chunk->pixels_raw));
            chunk->pixels.clear();
            chunk->pixels.shrink_to_fit();
            chunk->pixels_raw.clear();
            chunk->pixels_raw.shrink_to_fit();

            // these don't have to be accumulated
            width = chunk->width;
            height = chunk->height;

            last_frame = chunk->original_job_message->job->last_frame;
            if (last_frame) finished = true;
            frame_number = chunk->original_job_message->job->frame_number;
          }

          finished = process(width, height, pixels, pixels_raw, last_frame);

          save_images(gen->filename(),
                      rand_,
                      gen->get_seed(),
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
  if (finished && webserv) {
    if (!options().interactive) {
      webserv->stop();
    } else if (v && job_client) {
      auto fun = [&](std::shared_ptr<BitmapHandler> bmp_handler, seasocks::WebSocket* job_client) {
        // notifies client we're done w/o any data
        bmp_handler->callback(job_client);
      };
      if (webserv) {
        webserv->execute_bitmap(
            [&fun, job_client](std::shared_ptr<BitmapHandler> bmp_handler, std::shared_ptr<render_msg> job_msg) {
              fun(bmp_handler, job_client);
            },
            job_msg);
      }
    }
  }
}

void starcry::setup_server(const std::string& host) {
  if (options_.gui) gui = std::make_unique<sfml_window>();

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
    bitmaps[i] = std::make_shared<bitmap_wrapper>();
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

std::string starcry::serialize_shapes_to_json(std::vector<std::vector<data::shape>>& shapes) {
  json shapes_json = {};
  size_t index = 0;
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
    for (const auto& shape : shapes[shapes.size() - 1]) {
      // convert shape_type enum to string
      std::map<std::string, nlohmann::json> f = {
          {"type", data::shape_type_to_string(shape.type)},
          {"index", index},
          {"unique_id", shape.unique_id},
          {"id", shape.id},
          {"label", shape.label.empty() ? shape.id : shape.label},
          {"level", shape.level},
          {"gradient", shape.gradient_id_str},
          {"texture", shape.texture_id_str},
          {"x", shape.x},
          {"y", shape.y},
          {"x2", shape.x2},
          {"y2", shape.y2},
          {"radius", shape.radius},
          {"radius_size", shape.radius_size},
          {"velocity", shape.velocity},
          {"vel_x", shape.vel_x},
          {"vel_y", shape.vel_y},
          {"opacity", shape.opacity},
          {"dist", shape.dist},
          {"steps", shape.steps},
#ifdef DEBUG_NUM_SHAPES
          {"#", nums[shape.unique_id]},
          {"random_hash", shape.random_hash},
#else
          {"#", -1},
#endif
          {"time", shape.time},
      };
      json shape_json(f);
      shapes_json.push_back(shape_json);

      // TODO: script type
      index++;
    }
  }
  return shapes_json.dump();
}

const std::vector<int64_t> starcry::selected_ids_transitive(std::shared_ptr<job_message>& job) {
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
  if (selected_ids.size() > 0) {
    selected_ids_transitive = gen->get_transitive_ids(selected_ids);
  }
  return selected_ids_transitive;
}

void starcry::set_checkpoint(int frame) {
  checkpoints_.insert(frame);
}
