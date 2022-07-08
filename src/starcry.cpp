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

#include "cereal/archives/json.hpp"

#include "bitmap_wrapper.hpp"
#include "data/frame_request.hpp"
#include "data/video_request.hpp"
#include "framer.hpp"
#include "generator.h"
#include "network/render_client.h"
#include "network/render_server.h"
#include "rendering_engine.h"
#include "sfml_window.h"
#include "util/image_splitter.hpp"
#include "util/image_utils.h"
#include "util/logger.h"
#include "webserver.h"

#include "starcry/client_message_handler.h"
#include "starcry/metrics.h"
#include "starcry/server_message_handler.h"

// TODO: re-organize this somehow
#include <sys/prctl.h>
#include <random>

#include <inotify-cpp/FileSystemAdapter.h>
#include <inotify-cpp/NotifierBuilder.h>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

starcry::starcry(starcry_options &options, std::shared_ptr<v8_wrapper> &context)
    : context(context),
      options_(options),
      bitmaps({}),
      gen(nullptr),
      engines({}),
      system(std::make_shared<pipeline_system>(false)),
      cmds(system->create_queue("commands", 10)),
      jobs(system->create_queue("jobs", 10)),
      frames(system->create_queue("frames", 10)),
      pe(std::chrono::milliseconds(1000)),
      server_message_handler_(std::make_shared<server_message_handler>(*this)),
      client_message_handler_(std::make_shared<client_message_handler>(*this)),
      metrics_(std::make_shared<metrics>(options.notty || options.stdout_)),
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
  inotifypp::filesystem::path path("input");
  auto handleNotification = [&](inotify::Notification notification) {
    logger(DEBUG) << "File modified on disk: " << notification.path.string() << std::endl;
    logger(DEBUG) << "File of interest: " << script_ << std::endl;
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
  if (!std::filesystem::exists(path)) {
    std::cout << "path (" << path << ") does not exist" << std::endl;
    return;
  }
  notifier->watchPathRecursively(path)
      .onEvents(events, handleNotification)
      .onUnexpectedEvent(handleUnexpectedNotification);
  notifier_thread = std::thread([&]() {
    notifier->run();
  });
}

feature_settings &starcry::features() {
  return features_;
}

starcry_options &starcry::options() {
  return options_;
}

void starcry::set_script(const std::string &script) {
  script_ = script;
  if (metrics_) metrics_->set_script(script_);
  if (webserv) webserv->set_script(script_);
}

void starcry::add_image_command(std::shared_ptr<data::frame_request> req) {
  cmds->push(std::make_shared<instruction>(req));
  pe.run([=, this]() {
    if (webserv) {
      webserv->send_stats(system->get_stats());
      webserv->send_metrics(metrics_->to_json());
    }
  });
}

void starcry::add_video_command(std::shared_ptr<data::video_request> req) {
  cmds->push(std::make_shared<instruction>(req));
}

void starcry::render_job(
    size_t thread_num, rendering_engine &engine, const data::job &job, image &bmp, const data::settings &settings) {
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
                      options_.level == log_level::debug,
                      settings,
                      options_.debug || get_viewpoint().debug);
}

// MARK1 transform instruction into job (using generator)
void starcry::command_to_jobs(std::shared_ptr<instruction> cmd_def) {
  if (!gen) {
    context->recreate_isolate_in_this_thread();
    gen = std::make_shared<generator>(metrics_, context);
  }

  bool client_is_nullptr = false;
  if (cmd_def->type2 == instruction_type2::video) {
    auto &v = cmd_def->video_ref();
    gen->init(v.script(), options_.rand_seed, v.preview(), features_.caching);
    // Update current_frame if we fast-forward to a different offset frame.
    if (v.offset_frames() > 0) {
      current_frame = v.offset_frames();
    }
    double use_fps = gen->fps();
    if (!framer && options().output && cmd_def->video().output_file() != "/dev/null") {
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
      framer->set_log_callback([&](int level, const std::string &line) {
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
    if (v.client() == nullptr) {
      client_is_nullptr = true;
    }
  } else {
    metrics_->clear();

    const auto &f = cmd_def->frame();

    gen->init(f.script(), options_.rand_seed, f.preview(), features_.caching);
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
    if (f.client() == nullptr) {
      client_is_nullptr = true;
    }
  }
  if (client_is_nullptr) {
    cmds->check_terminate();
    jobs->check_terminate();
  }
}

// MARK1 render job using renderer and transform into render_msg
std::shared_ptr<render_msg> starcry::job_to_frame(size_t i, std::shared_ptr<job_message> job_msg) {
  auto &job = *job_msg->job;

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

  // handle videos
  if (job_msg->original_instruction->type2 == instruction_type2::video) {
    const auto &v = job_msg->original_instruction->video();
    if (v.client() != nullptr) {
      return nullptr;
    }
    auto transfer_pixels = pixels_vec_to_pixel_data(bmp.pixels(), gen->settings().dithering);

    auto msg = std::make_shared<render_msg>(job_msg);
    msg->set_pixels(transfer_pixels);
    if (v.raw_video()) {
      msg->set_raw(bmp.pixels());
    }
    return msg;
  }

  // handle frames
  const auto &f = job_msg->original_instruction->frame();
  auto msg = std::make_shared<render_msg>(job_msg);
  job.job_number = std::numeric_limits<uint32_t>::max();

  if (f.raw_image() || get_viewpoint().raw || get_viewpoint().save) {
    msg->set_raw(bmp.pixels());
  }

  if (f.raw_bitmap()) {
    auto transfer_pixels = pixels_vec_to_pixel_data(bmp.pixels(), gen->settings().dithering);
    msg->set_pixels(transfer_pixels);
  }

  if (job_msg->original_instruction->frame_ptr() && !job_msg->original_instruction->frame_ptr()->client() &&
      f.compressed_image()) {
    png::image<png::rgba_pixel> image(job.width, job.height);
    copy_to_png(bmp.pixels(), job.width, job.height, image, gen->settings().dithering);
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
    json shapes_json = {};
    auto &shapes = job_msg->job->shapes;
    size_t index = 0;
    if (!shapes.empty()) {
#define DEBUG_NUM_SHAPES
#ifdef DEBUG_NUM_SHAPES
      std::unordered_map<int64_t, int64_t> nums;
      for (size_t i = 0; i < shapes.size(); i++) {
        for (const auto &shape : shapes[i]) {
          nums[shape.unique_id]++;
        }
      }
#endif
      for (const auto &shape : shapes[shapes.size() - 1]) {
        if (shape.type == data::shape_type::circle) {
          json circle = {
              {"index", index},
              {"unique_id", shape.unique_id},
              {"id", shape.id},
              {"label", shape.label.empty() ? shape.id : shape.label},
              {"level", shape.level},
              {"gradient", shape.gradient_id_str},
              {"type", "circle"},
              {"x", shape.x},
              {"y", shape.y},
              {"vel_x", shape.vel_x},
              {"vel_y", shape.vel_y},
#ifdef DEBUG_NUM_SHAPES
              {"#", nums[shape.unique_id]},
              {"random_hash", shape.random_hash},
#else
              {"#", -1},
#endif
              {"time", shape.time},
          };
          shapes_json.push_back(circle);
        }
        if (shape.type == data::shape_type::line) {
          json line = {
              {"index", index},
              {"unique_id", shape.unique_id},
              {"id", shape.id},
              {"label", shape.label.empty() ? shape.id : shape.label},
              {"level", shape.level},
              {"gradient", shape.gradient_id_str},
              {"type", "line"},
              {"x", shape.x},
              {"y", shape.y},
              {"x2", shape.x2},
              {"y2", shape.y2},
#ifdef DEBUG_NUM_SHAPES
              {"#", nums[shape.unique_id]},
              {"random_hash", shape.random_hash},
#else
              {"#", -1},
#endif
              {"time", shape.time},
          };
          shapes_json.push_back(line);
        }
        // TODO: script type
        index++;
      }
    }
    auto str = shapes_json.dump();
    msg->set_buffer(str);
  }

  msg->set_width(get_viewpoint().canvas_w ? get_viewpoint().canvas_w : job.width);
  msg->set_height(get_viewpoint().canvas_h ? get_viewpoint().canvas_h : job.height);

  // TODO: below belongs somewhere else
  features().caching = get_viewpoint().caching;
  return msg;
}

// MARK1 handle the render msg, create into video or whatever
void starcry::handle_frame(std::shared_ptr<render_msg> job_msg) {
  auto instr = job_msg->original_job_message->original_instruction;
  std::shared_ptr<data::frame_request> f = instr->frame_ptr();
  std::shared_ptr<data::video_request> v = instr->video_ptr();
  auto type = job_msg->original_job_message->original_instruction->type2;
  bool finished = false;
  auto job = *job_msg->original_job_message->job;
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
                     std::vector<uint32_t> &pixels,
                     std::vector<data::color> &pixels_raw,
                     bool last_frame) {
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
      return last_frame;
    }
    std::swap(job_msg->pixels, pixels);
    std::swap(job_msg->pixels_raw, pixels_raw);

    if (job_client != nullptr && f && (f->raw_bitmap() || f->raw_image())) {
      auto fun = [&](std::shared_ptr<BitmapHandler> bmp_handler, std::shared_ptr<render_msg> job_msg) {
        std::string buffer;
        for (const auto &i : job_msg->pixels) {
          buffer.append((char *)&i, sizeof(i));
        }
        bmp_handler->callback(job_client, buffer, job_msg->width, job_msg->height);
      };
      if (webserv) webserv->execute_bitmap(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
    }

    if (job_client != nullptr && f && f->compressed_image()) {
      auto fun = [&](std::shared_ptr<ImageHandler> image_handler, std::shared_ptr<render_msg> job_msg) {
        image_handler->callback(job_client, job_msg->buffer);
      };
      if (webserv) {
        webserv->execute_image(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
      }
    }

    if (job_client != nullptr && f && f->metadata_objects()) {
      job_msg->ID = webserv->get_client_id(job_client);
      auto fun = [&](std::shared_ptr<ObjectsHandler> objects_handler, std::shared_ptr<render_msg> job_msg) {
        if (objects_handler->_links.find(job_msg->ID) != objects_handler->_links.end()) {
          auto con = objects_handler->_links[job_msg->ID];  // find con that matches ID this msg is from
          objects_handler->callback(con, job_msg->buffer);
        }
      };
      if (webserv) webserv->execute_objects(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
    }

    if (job_client != nullptr && f && f->renderable_shapes()) {
      auto fun = [&](std::shared_ptr<ShapesHandler> shapes_handler, std::shared_ptr<render_msg> job_msg) {
        shapes_handler->callback(job_client, job_msg->buffer);
      };
      if (webserv) {
        webserv->execute_shapes(std::bind(fun, std::placeholders::_1, std::placeholders::_2), job_msg);
      }
    }

    return finished;
  };

  buffered_frames[job.job_number].push_back(job_msg);

  if (type == instruction_type2::image) {
    if (f->renderable_shapes() || f->metadata_objects()) {
      current_frame = job.frame_number;
    }
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
    if (pos->second.size() == pos->second[0]->original_job_message->job->num_chunks) {
      std::sort(pos->second.begin(), pos->second.end(), [](const auto &lh, const auto &rh) {
        return lh->original_job_message->job->chunk < rh->original_job_message->job->chunk;
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
        last_frame = chunk->original_job_message->job->last_frame;
        frame_number = chunk->original_job_message->job->frame_number;
      }

      finished = process(width, height, pixels, pixels_raw, last_frame);

      if (job.job_number == std::numeric_limits<uint32_t>::max()) {
        save_images(pixels_raw, width, height, frame_number, true, true, job.output_file);
        if (job.last_frame) {
          finished = true;
        }
      } else {
        save_images(pixels_raw, width, height, frame_number, true, true, job.output_file);
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

void starcry::setup_server() {
  if (options_.gui) gui = std::make_unique<sfml_window>();

  system->spawn_consumer<instruction>(
      "generator", std::bind(&starcry::command_to_jobs, this, std::placeholders::_1), cmds);

  if (options_.enable_remote_workers) {
    renderserver = std::make_shared<render_server>(jobs, frames);
    renderserver->run(std::bind(&client_message_handler::on_client_message,
                                *client_message_handler_,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3,
                                std::placeholders::_4));
  }

  for (size_t i = 0; i < options_.num_worker_threads; i++) {
    metrics_->register_thread(i, fmt::format("L{}", i));
    engines[i] = std::make_shared<rendering_engine>();
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
}

void starcry::run_server() {
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

void starcry::run_client(const std::string &host) {
  render_client client(host);
  rendering_engine engine;
  bitmap_wrapper bitmap;

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
