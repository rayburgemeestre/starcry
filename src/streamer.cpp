/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "actors/streamer.h"

#include "benchmark.h"
#include <bitset>
#include "data/job.hpp"
#include "output_action.hpp"

// public
using start            = atom_constant<atom("start     ")>;
using render_frame     = atom_constant<atom("render_fra")>;
using show_stats       = atom_constant<atom("show_stats")>;
using need_frames      = atom_constant<atom("need_frame")>;
using debug            = atom_constant<atom("debug     ")>;

// external
using del_job          = atom_constant<atom("del_job   ")>;
using start_rendering  = atom_constant<atom("start_rend")>;
using stop_rendering   = atom_constant<atom("stop_rende")>;

// internal
using process_queue    = atom_constant<atom("process_qu")>;
using ready            = atom_constant<atom("ready     ")>;

auto benchmark_class2 = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter2 = static_cast<MeasureInterval &>(*benchmark_class2.get());

size_t current_frame2 = 0; // initialize with a start ?

using namespace std;
vector<tuple<size_t,size_t,size_t,bool, vector<uint32_t>>> fake_buffer, matches;
std::optional<size_t> last_frame_streamed;


// TODO; add state to streamer object
// TODO; refactor this to regular objects.. this doesn't make sense..
// as a temporary workaround putting stuff inside a unique_ptr...
unique_ptr<output_action<ffmpeg_h264_encode>> ffmpeg;
unique_ptr<output_action<allegro5_window>> allegro5;
unique_ptr<output_aggregate> outputs;

//#define DEBUG_FRAMES

#ifdef DEBUG_FRAMES
#include "rendering_engine.hpp"
#endif

bool process_buffer(event_based_actor* self, size_t frame_number, size_t num_chunks, uint32_t canvas_w, uint32_t canvas_h) {
    auto frame_number_matches = [&](auto &tpl) { return std::get<0>(tpl) == frame_number; };
    auto sort_by_chunk        = [&](auto &tpl, auto &tpl2) { return std::get<1>(tpl) < std::get<1>(tpl2); };

    matches.clear();
    copy_if(fake_buffer.begin(), fake_buffer.end(), back_inserter(matches), frame_number_matches);

    if (matches.size() == num_chunks) {
        counter2.measure();

        sort(matches.begin(), matches.end(), sort_by_chunk);

        // we split the image horizontally so we can just concat all pixels here
        vector<uint32_t> pixels_all;
        for (auto &tpl : matches) {
            pixels_all.insert(pixels_all.end(), std::get<4>(tpl).begin(), std::get<4>(tpl).end() );
        }
#ifdef DEBUG_FRAMES
        cout << "streamer completed frame: " << current_frame2 << " number of pixels equal to: " << pixels_all.size()
        << " number of matches = " << matches.size() << endl; // debug
        stringstream ss;
        static int i = 1000;
        ss << "frame" << i++;
        //rendering_engine eng;
        //auto bmp = eng.unserialize_bitmap(pixels_all, 1280, 720);
        //eng.write_image(bmp, ss.str());
        //al_destroy_bitmap(bmp);
#endif
        outputs->add_frame(canvas_w, canvas_h, pixels_all); // encoder has it's own frameNumber variable.

        fake_buffer.erase(std::remove_if(fake_buffer.begin(),
                                         fake_buffer.end(),
                                         frame_number_matches),
                          fake_buffer.end());
        if (last_frame_streamed && *last_frame_streamed == frame_number) {
            outputs->finalize();
            aout(self) << "streamer completed frames: " << current_frame2 << ", with FPS: " << (1000.0 / counter2.mean())
                       << " +/- " << counter2.stderr() << endl;
            self->quit(exit_reason::user_shutdown);
        }
        return true;
    };
    return false;
}

static size_t min_items_in_streamer_queue = 20;
static size_t max_items_in_streamer_queue = 50;

size_t num_pixels = 0;

behavior streamer(event_based_actor* self, const caf::actor &job_storage, int render_window_at, string output_file, uint32_t settings) {
    ffmpeg   = std::make_unique<output_action<ffmpeg_h264_encode>>();
    allegro5 = std::make_unique<output_action<allegro5_window>>(self->system());
    outputs  = std::make_unique<output_aggregate>(*ffmpeg.get(), *allegro5.get());
    if (bitset<32>(settings).test(0)) {
        ffmpeg->enable(true);
        ffmpeg->set_filename(output_file);
    }
    if (bitset<32>(settings).test(1)) {
        allegro5->enable(true);
    }

    counter2.setDescription("fps");
    counter2.startHistogramAtZero(true);
    return {
        [=, &num_pixels](render_frame, struct data::job &job, vector<uint32_t> &pixels, const caf::actor &renderer) {
            num_pixels = job.canvas_w * job.canvas_h;
            if (job.last_frame)
                last_frame_streamed = std::make_optional(job.frame_number);

            ffmpeg->set_bitrate(job.bitrate); // todo put in initialize?
            outputs->initialize(job.canvas_w, job.canvas_h, self, render_window_at);

            fake_buffer.push_back(make_tuple(job.frame_number, job.chunk, job.num_chunks, job.last_frame, pixels)); // needs to become an object later
            while (process_buffer(self, current_frame2, job.num_chunks, job.canvas_w, job.canvas_h))
                current_frame2++;

            if (self->mailbox().count() > max_items_in_streamer_queue) {
                // instruct renderer to stop until we have processed our queue a bit further.
                // renderer itself will periodically check with us to see if it's okay to resume again (via need_frames)
                self->send(renderer, stop_rendering::value);
            }

            self->send(job_storage, del_job::value, job.job_number);
        },
        [=](start, caf::actor &) {
            aout(self) << "ERROR: streamer no longer responding to start atom." << endl;
        },
        [=](need_frames) {
            return make_message(need_frames::value, self->mailbox().count() < min_items_in_streamer_queue);
        },
        [=, &num_pixels](show_stats) {
            auto fps = (1000.0 / counter2.mean());
            aout(self) << "streamer at frame: " << current_frame2 << ", with FPS: " << fps
                       << " +/- " << counter2.stderr() << " (" << ((num_pixels * sizeof(uint32_t) * fps) / 1024 / 1024) << " MiB/sec)" << endl;
        },
        [=, &num_pixels](show_stats, string renderer_stats) {
            auto fps = (1000.0 / counter2.mean());
            aout(self) << "streamer at frame: " << current_frame2 << ", with FPS: " << fps
                       << " +/- " << counter2.stderr() << " (" << ((num_pixels * sizeof(uint32_t) * fps) / 1024 / 1024) << " MiB/sec), "
                       << renderer_stats << endl;
        },
        [=](debug) {
            aout(self) << "streamer mailbox = " << self->mailbox().count() << endl;
        }
    };
}
