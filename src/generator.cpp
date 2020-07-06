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
#include "v8_wrapper.hpp"

std::shared_ptr<v8_wrapper> context;
std::unique_ptr<assistant_> assistant;

generator::generator() {}

void generator::init() {
  use_stdin = false;
  const std::string filename = "input/test.js";

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

// TODO: this doesn't belong here!!!! Just to test..
#include "allegro5/allegro5.h"
#include "rendering_engine_wrapper.h"
#include "framer.hpp"

void generator::on_write_frame(data::job &job) {
  static int counter = 1;

  a(std::cout) << "on write frame" << std::endl;
  a(std::cout) << job.shapes.size() << std::endl;

  static rendering_engine_wrapper engine;
  static ALLEGRO_BITMAP *bitmap = nullptr;
  if (counter == 1) {
    engine.initialize();
    bitmap = al_create_bitmap(job.width, job.height);
  }

  // job.background_color.r = 1.0;
  job.offset_x = 0; // ??
  job.offset_y = 0; // ?? not initialized correctly somehow.. we'll figure it out, somewhere copied, or whatever

  engine.render(bitmap,
                job.background_color,
                job.shapes,
                job.offset_x,
                job.offset_y,
                job.canvas_w,
                job.canvas_h,
                job.width,
                job.height,
                job.scale);

//  engine.write_image(bitmap, "output");

  // another test because I am impatient
  static auto framer = std::make_shared<frame_streamer>("test.h264",
                                                        bitrate,
                                                        25, // fps,
                                                        job.canvas_w,
                                                        job.canvas_h,
                                                        frame_streamer::stream_mode::FILE);
  if (counter == 1) {
    std::cout << "initializing framer" << std::endl;
    framer->run();
  }

  auto pixels = engine.serialize_bitmap2(bitmap, job.width, job.height);
  framer->add_frame(pixels);

  if (counter == 250) {
    framer->finalize();
  }
  counter++;
}

