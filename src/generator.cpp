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

generator::generator(std::function<void(size_t, int, int, int)> on_initialized,
                     std::function<void(const data::job &)> on_new_job)
    : on_initialized(on_initialized), on_new_job(on_new_job) {}

void generator::init(const std::string &filename) {
  use_stdin = false;

  // initialize V8
  context = std::make_shared<v8_wrapper>(filename);
  std::ifstream stream(filename.c_str());
  if (!stream && filename != "-") {
    throw std::runtime_error("could not locate file " + filename);
  }

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

  std::istreambuf_iterator<char> begin(filename == "-" ? std::cin : stream), end;
  context->run("var current_frame = 0;");
  context->run(std::string(begin, end));
  if (context->run<bool>("typeof initialize != 'undefined'")) {
    call_print_exception("initialize");
  }

  // initialize
  bitrate = context->run<double>("typeof bitrate != 'undefined' ? bitrate : (500 * 1024 * 8)");
  use_stdin = context->run<bool>("typeof stdin != 'undefined' ? stdin : false");
  use_fps = context->run<size_t>("typeof fps != 'undefined' ? fps : 25");
  realtime = context->run<bool>("typeof realtime != 'undefined' ? realtime : false");

  // start
  // renderer_ptr = renderer;
  max_jobs_queued_for_renderer = max_jobs_queued_for_renderer;
  num_chunks = num_chunks;
  canvas_w = context->run<uint32_t>("typeof canvas_w != 'undefined' ? canvas_w : " + std::to_string(canvas_w));
  canvas_h = context->run<uint32_t>("typeof canvas_h != 'undefined' ? canvas_h : " + std::to_string(canvas_h));

  // initialize job object
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
  job.scale = context->run<double>("typeof scale != 'undefined' ? scale : 1.0");
  job.compress = false;  // compress;
  job.save_image = false;
  assistant->max_frames = context->run<size_t>("typeof max_frames != 'undefined' ? max_frames : 250");
  assistant->realtime = context->run<bool>("typeof realtime != 'undefined' ? realtime : false");

  on_initialized(bitrate, canvas_w, canvas_h, use_fps);
}

void generator::generate_frame() {
  a(std::cout) << "gen: " << assistant->current_frame << std::endl;
  // for (int i=0; i<600; i++) {
  //  assistant->the_job.shapes.clear();
  assistant->the_previous_job = assistant->the_job;
  context->run("current_frame = " + std::to_string(assistant->current_frame) + ";");
  call_print_exception("next");
  // assistant->the_job.shapes[0] = assistant->the_previous_job.shapes[0];
  // assistant->the_job.shapes[0].reset( assistant->the_previous_job.shapes[0] );
  // }
  write_frame_fun();
}

void call_print_exception(const std::string &fn) {
  try {
    context->call(fn);
  } catch (std::runtime_error &ex) {
    a(std::cout) << "Runtime error caused during execution of " << fn << "() in javascript:" << endl
                 << std::string(ex.what()) << endl;
  }
}

template <typename T>
void call_print_exception(std::string fn, T arg) {
  try {
    context->call(fn, arg);
  } catch (std::runtime_error &ex) {
    a(std::cout) << "Runtime error caused during execution of " << fn << "(..) in javascript:" << endl
                 << std::string(ex.what()) << endl;
  }
}

void generator::on_output_line(const std::string &s) {}

void generator::on_write_frame(data::job &job) {
  a(std::cout) << "on write frame" << std::endl;
  a(std::cout) << job.shapes.size() << std::endl;

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

    // job is now done to be rendered?? at this point
    on_new_job(job);

    counter++;
  }
}
