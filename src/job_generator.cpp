/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <fstream>
#include <memory>

#include "actors/job_generator.h"
#include "atom_types.h"
#include "benchmark.h"
#include "common.h"
#include "data/job.hpp"
#include "job_cache.h"
#include "primitives.h"
#include "scripting.h"
#include "util/assistant.h"
#include "util/image_splitter.hpp"
#include "v8_wrapper.hpp"

using namespace std;

shared_ptr<v8_wrapper> context;
unique_ptr<assistant_> assistant;

behavior job_generator(stateful_actor<job_generator_data> *self,
                       const string &filename,
                       uint32_t canvas_w,
                       uint32_t canvas_h,
                       bool use_stdin,
                       bool rendering_enabled,
                       bool compress) {
  self->state.use_stdin = use_stdin;

  // initialize fps counter
  self->state.fps_counter = std::make_shared<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
  MeasureInterval &fps_counter = *self->state.fps_counter;
  fps_counter.setDescription("fps");
  fps_counter.startHistogramAtZero(true);

  // initialize V8
  context = make_shared<v8_wrapper>(filename);
  try {
    ifstream stream(filename.c_str());
    if (!stream && filename != "-") {
      throw runtime_error("could not locate file " + filename);
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

    istreambuf_iterator<char> begin(filename == "-" ? std::cin : stream), end;
    context->run("var current_frame = 0;");
    context->run(std::string(begin, end));
    if (context->run<bool>("typeof initialize != 'undefined'")) {
      call_print_exception(self, "initialize");
    }
  } catch (exception &ex) {
    cout << ex.what() << endl;
  }

  // return actor behavior
  return {
      [=](initialize) -> message {  // by main
        self->state.bitrate = context->run<double>("typeof bitrate != 'undefined' ? bitrate : (500 * 1024 * 8)");
        self->state.use_stdin = context->run<bool>("typeof stdin != 'undefined' ? stdin : false");
        self->state.use_fps = context->run<size_t>("typeof fps != 'undefined' ? fps : 25");
        self->state.realtime = context->run<bool>("typeof realtime != 'undefined' ? realtime : false");
        return make_message(self->state.bitrate, self->state.use_stdin, self->state.use_fps, self->state.realtime);
      },
      [=](start, size_t max_jobs_queued_for_renderer, size_t num_chunks, actor &renderer) {  // by main
        // further initialize state
        self->state.renderer_ptr = renderer;
        self->state.max_jobs_queued_for_renderer = max_jobs_queued_for_renderer;
        self->state.num_chunks = num_chunks;
        self->state.canvas_w =
            context->run<uint32_t>("typeof canvas_w != 'undefined' ? canvas_w : " + to_string(canvas_w));
        self->state.canvas_h =
            context->run<uint32_t>("typeof canvas_h != 'undefined' ? canvas_h : " + to_string(canvas_h));

        // initialize job object
        assistant = std::make_unique<assistant_>(self);
        auto &job = assistant->the_job;
        job.width = self->state.canvas_w;
        job.height = self->state.canvas_h;
        job.frame_number = assistant->current_frame;
        job.rendered = false;
        job.last_frame = false;
        job.num_chunks = self->state.num_chunks;
        job.canvas_w = self->state.canvas_w;
        job.canvas_h = self->state.canvas_h;
        job.scale = context->run<double>("typeof scale != 'undefined' ? scale : 1.0");
        job.compress = compress;
        job.save_image = false;
        assistant->max_frames = context->run<size_t>("typeof max_frames != 'undefined' ? max_frames : 250");
        assistant->realtime = context->run<bool>("typeof realtime != 'undefined' ? realtime : false");
        if (!self->state.use_stdin || assistant->realtime) {
          for (int i = 0; i < self->state.max_jobs_queued_for_renderer; i++) {
            // we call into V8 ourselves to get frames
            // for (int i=0; i<600; i++) {
            //  assistant->the_job.shapes.clear();
            assistant->the_previous_job = assistant->the_job;
            context->run("current_frame = " + to_string(assistant->current_frame) + ";");
            call_print_exception(self, "next");
            // assistant->the_job.shapes[0] = assistant->the_previous_job.shapes[0];
            // assistant->the_job.shapes[0].reset( assistant->the_previous_job.shapes[0] );
            // }
            write_frame_fun();
          }
        }
      },
      [=](input_line, string line) {  // by stdin_reader
        call_print_exception(self, "input", line);
        self->state.lines_received++;
      },
      [=](no_more_input) {  // by stdin_reader
        assistant->the_job.last_frame = true;
        call_print_exception(self, "close");
      },
      [=](checkpoint, const actor &sender) {  // by stdin_reader
        // communicate back to stdin_reader how many lines we have received, and whether we have enough
        // jobs delivered to renderer for now (meaning the stdin_reader can pause providing lines for us)
        auto &state = self->state;
        self->state.has_max_jobs_queued_for_renderer =
            (self->state.jobs_queued_for_renderer >= self->state.max_jobs_queued_for_renderer);
        self->send(sender, checkpoint_v, state.lines_received, state.has_max_jobs_queued_for_renderer);
      },
      [=](job_processed) {  // by renderer
        self->state.jobs_queued_for_renderer--;

        if (!self->state.use_stdin || assistant->realtime) {
          for (int64_t i = self->state.jobs_queued_for_renderer; i < self->state.max_jobs_queued_for_renderer; i++) {
            self->state.jobs_queued_for_renderer++;
            self->delayed_send(self, std::chrono::milliseconds(8), next_frame_v);
          }
        }
      },
      [=](write_frame, data::job job) {  // from scripting (V8)
        util::ImageSplitter<uint32_t> imagesplitter{self->state.canvas_w, self->state.canvas_h};
        // We always split horizontal, so we simply can concat the pixels later
        const auto rectangles =
            imagesplitter.split(self->state.num_chunks, util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
        for (size_t i = 0, counter = 1; i < rectangles.size(); i++) {
          job.width = rectangles[i].width();
          job.height = rectangles[i].height();
          job.offset_x = rectangles[i].x();
          job.offset_y = rectangles[i].y();
          job.job_number = self->state.current_job++;
          job.scale = context->run<double>("typeof scale != 'undefined' ? scale : 1.0");
          job.chunk = counter;
          if (rendering_enabled) {
            self->send(*self->state.renderer_ptr, add_job_v, job);
          } else {
            self->send(self, next_frame_v);
          }
          counter++;
        }

        self->state.fps_counter->measure();
        job.shapes.clear();
        if (job.last_frame) {
          aout(self) << "job_generator: there are no more jobs to be generated." << endl;
          self->state.jobs_queued_for_renderer = numeric_limits<int64_t>::max();
          // self->become(nothing); don't do this, v8 might be calling into us still..
          if (!rendering_enabled) {
            // this actor shuts down the system in that case
            //    self->quit(exit_reason::user_shutdown);
          }
          // this for some reason causes problems.
          self->quit(exit_reason::user_shutdown);
        }
      },
      [=](output_line, string line) {  // scripting (V8)
        aout(self) << line << endl;
      },
      [=](next_frame) {
        context->run("current_frame = " + to_string(assistant->current_frame) + ";");
        call_print_exception(self, "next");
        // restore the state test
        // if (assistant->the_previous_job.shapes.size() > 0)
        //  assistant->the_job.shapes[0] = assistant->the_previous_job.shapes[0];
        assistant->the_previous_job = assistant->the_job;
        if (!self->state.use_stdin || assistant->realtime) {
          write_frame_fun();
        }
      },
      [=](debug) {                                                          // main
        aout(self) << "job_generator mailbox = " << self->mailbox().count() /*<< " " << self->mailbox().counter()*/
                   << endl;
      },
      [=](show_stats) {  // main
        aout(self) << "generator at frame: " << assistant->current_frame
                   << ", with FPS: " << (1000.0 / self->state.fps_counter->mean()) << " +/- "
                   << self->state.fps_counter->stderr() << " mailbox: " << self->mailbox().count() << endl;
      },
      [=](terminate_) {  // main
        aout(self) << "terminating.." << endl;
        self->quit(exit_reason::user_shutdown);
      },
  };
}

assistant_::assistant_(stateful_actor<job_generator_data> *job_gen)
    : job_generator(job_gen), cache(std::make_unique<job_cache>()) {}

void call_print_exception(event_based_actor *self, string fn) {
  try {
    context->call(fn);
  } catch (std::runtime_error &ex) {
    aout(self) << "Runtime error caused during execution of " << fn << "() in javascript:" << endl
               << string(ex.what()) << endl;
    self->quit(exit_reason::user_shutdown);
  }
}

template <typename T>
void call_print_exception(event_based_actor *self, string fn, T arg) {
  try {
    context->call(fn, arg);
  } catch (std::runtime_error &ex) {
    aout(self) << "Runtime error caused during execution of " << fn << "(..) in javascript:" << endl
               << string(ex.what()) << endl;
    self->quit(exit_reason::user_shutdown);
  }
}
