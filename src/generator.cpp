/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "generator.h"

#include "util/a.hpp"

#include <scripting.h>
#include <memory>

#include "primitives.h"
#include "scripting.h"
#include "util/assistant.h"
#include "util/image_splitter.hpp"
#include "v8_wrapper.hpp"

std::shared_ptr<v8_wrapper> context;
std::unique_ptr<assistant_> assistant;

// TODO: find a better home for this?
assistant_::assistant_() : cache(std::make_unique<job_cache>()) {}

generator::generator(std::function<void(size_t, int, int, int)> on_initialized,
                     std::function<bool(const data::job &)> on_new_job,
                     std::optional<size_t> custom_max_frames)
    : on_initialized(on_initialized), on_new_job(on_new_job), custom_max_frames(custom_max_frames) {}

generator::~generator() {
  // The following should stay commented, successive instances of this generator class
  // should re-use v8, or you get weird results. This is the reason why they are stored
  // outside of this class in the global space instead.
  // context = nullptr;
  // assistant = nullptr;
}

void generator::init(const std::string &filename) {
  use_stdin = false;  // TODO: now hardcoded

  if (context == nullptr) {
    context = std::make_shared<v8_wrapper>(filename);

    context->add_class(shape::add_to_context);
    context->add_class(circle::add_to_context);
    context->add_class(rectangle::add_to_context);
    context->add_class(line::add_to_context);
    context->add_class(color::add_to_context);
    context->add_class(pos::add_to_context);
    context->add_class(gradient::add_to_context);

    context->add_fun("version", &get_version);
    context->add_fun("output", &output);
    context->add_fun("write_frame", &write_frame_fun);
    context->add_fun("close", &close_fun);
    context->add_fun("add_text", &add_text);
    context->add_fun("set_background_color", &set_background_color);
    context->add_fun("add_circle", &add_circle);
    context->add_fun("add_line", &add_line);
    context->add_fun("rand", &rand_fun);
    // TODO: context->add_fun("add_rectangle", &add_rectangle);

    context->add_include_fun();
  }

  // evaluate input script file in V8 context
  std::ifstream stream(filename.c_str());
  if (!stream && filename != "-") {
    throw std::runtime_error("could not locate file " + filename);
  }
  std::istreambuf_iterator<char> begin(filename == "-" ? std::cin : stream), end;
  context->run("var current_frame = 0;");
  context->run(std::string(begin, end));
  if (context->run<bool>("typeof initialize != 'undefined'")) {
    call_print_exception("initialize");
  }

  // read some configuration from V8 context
  bitrate = context->run<double>("typeof bitrate != 'undefined' ? bitrate : (500 * 1024 * 8)");
  use_stdin = context->run<bool>("typeof stdin != 'undefined' ? stdin : false");
  use_fps = context->run<size_t>("typeof fps != 'undefined' ? fps : 25");
  realtime = context->run<bool>("typeof realtime != 'undefined' ? realtime : false");
  canvas_w = context->run<uint32_t>("typeof canvas_w != 'undefined' ? canvas_w : " + std::to_string(canvas_w));
  canvas_h = context->run<uint32_t>("typeof canvas_h != 'undefined' ? canvas_h : " + std::to_string(canvas_h));
  realtime = context->run<bool>("typeof realtime != 'undefined' ? realtime : false");
  auto scale = context->run<double>("typeof scale != 'undefined' ? scale : 1.0");
  auto max_frames = context->run<size_t>("typeof max_frames != 'undefined' ? max_frames : 250");

  // prepare job object
  assistant = std::make_unique<assistant_>();
  assistant->generator = this;
  auto &job = assistant->the_job;
  job.width = canvas_w;
  job.height = canvas_h;
  job.frame_number = assistant->current_frame;
  job.rendered = false;
  job.last_frame = false;
  job.num_chunks = num_chunks;
  job.canvas_w = canvas_w;
  job.canvas_h = canvas_h;
  job.scale = scale;
  job.compress = false;  // TODO: hardcoded
  job.save_image = false;
  assistant->max_frames = max_frames;
  if (custom_max_frames) {
    assistant->max_frames = *custom_max_frames;
  }
  assistant->realtime = realtime;

  // fully initialized now
  on_initialized(bitrate, canvas_w, canvas_h, use_fps);
}

bool generator::generate_frame() {
  assistant->the_previous_job = assistant->the_job;
  context->run("current_frame = " + std::to_string(assistant->current_frame) + ";");
  call_print_exception("next");
  // assistant->the_job.shapes[0] = assistant->the_previous_job.shapes[0];
  // assistant->the_job.shapes[0].reset( assistant->the_previous_job.shapes[0] );
  return !write_frame_fun();
}

void call_print_exception(const std::string &fn) {
  try {
    context->call(fn);
  } catch (std::runtime_error &ex) {
    a(std::cout) << "Runtime error caused during execution of " << fn << "() in javascript:" << std::endl
                 << std::string(ex.what()) << std::endl;
  }
}

template <typename T>
void call_print_exception(std::string fn, T arg) {
  try {
    context->call(fn, arg);
  } catch (std::runtime_error &ex) {
    a(std::cout) << "Runtime error caused during execution of " << fn << "(..) in javascript:" << std::endl
                 << std::string(ex.what()) << std::endl;
  }
}

void generator::on_output_line(const std::string &s) {}

void generator::on_write_frame(data::job &job) {
  util::ImageSplitter<uint32_t> imagesplitter{canvas_w, canvas_h};
  // We always split horizontal, so we simply can concat the pixels later
  const auto rectangles =
      imagesplitter.split(1 /* number of chunks */, util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
  for (size_t i = 0, counter = 1; i < rectangles.size(); i++) {
    job.width = rectangles[i].width();
    job.height = rectangles[i].height();
    job.offset_x = rectangles[i].x();
    job.offset_y = rectangles[i].y();
    job.job_number = current_job++;
    job.chunk = counter;
    if (!on_new_job(job)) {
      job.last_frame = true;
    }
    counter++;
  }
}
