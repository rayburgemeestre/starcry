/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "actors/streamer.h"
#include "benchmark.h"
#include "data/job.hpp"
#include "data/pixels.hpp"
#include "streamer_output/ffmpeg_encode.h"
#include "streamer_output/ffmpeg_stream.h"
#include "streamer_output/ffmpeg_stream_hls.h"
#include "streamer_output/allegro5_window.h"
#include "util/compress_vector.h"
#include "util/remote_actors.h"
#include "caf/io/all.hpp"
#include <bitset>

// public
using start            = atom_constant<atom("start     ")>;
using render_frame     = atom_constant<atom("render_fra")>;
using show_stats       = atom_constant<atom("show_stats")>;
using need_frames      = atom_constant<atom("need_frame")>;
using debug            = atom_constant<atom("debug     ")>;
using initialize       = atom_constant<atom("initialize")>;
using checkpoint       = atom_constant<atom("checkpoint")>;
using terminate_       = atom_constant<atom("terminate ")>;

// external
using del_job          = atom_constant<atom("del_job   ")>;
using stop_rendering   = atom_constant<atom("stop_rende")>;
using streamer_ready   = atom_constant<atom("streamer_r")>;

// internal
using ready            = atom_constant<atom("ready     ")>;
using recheck_test     = atom_constant<atom("recheck_te")>;

using namespace std;

bool all_frame_chunks_present(set<rendered_job> &rendered_jobs_set, size_t frame_number, size_t num_chunks) {
    size_t num_chunks_for_frame_available = 0;
    for (const auto &job : rendered_jobs_set) {
        if (job.frame_number != frame_number)
            return false;
        if (++num_chunks_for_frame_available == num_chunks)
            return true;
    }
    return false;
}

bool process_buffer(stateful_actor<streamer_data> *self, const actor &renderer, size_t frame_num, const data::job &j) {
    size_t frame_number = frame_num;
    size_t num_chunks = j.num_chunks;
    size_t canvas_w = j.canvas_w;
    uint32_t canvas_h = j.canvas_h;
    auto & rendered_jobs = self->state.rendered_jobs_set;

    if (!all_frame_chunks_present(rendered_jobs, frame_number, num_chunks))
        return false;

    // we split the image horizontally so we can just concat all pixels here
    vector<uint32_t> pixels_all;
    pixels_all.reserve(canvas_w * canvas_h);
    for (size_t i=0; i<num_chunks; i++) {
        auto &pixels = rendered_jobs.cbegin()->pixels;
        pixels_all.insert(pixels_all.end(), pixels.cbegin(), pixels.cend() );
        rendered_jobs.erase(rendered_jobs.begin());
    }
    //cout << "streamer completed frame: " << self->state.current_frame << " number of pixels equal to: "
    //     << pixels_all.size() << endl; // debug
    if (self->state.ffmpeg)
        self->state.ffmpeg->add_frame(pixels_all);
    if (self->state.ffmpeg_stream_flv)
        self->state.ffmpeg_stream_flv->add_frame(pixels_all);
    if (self->state.ffmpeg_stream_hls)
        self->state.ffmpeg_stream_hls->add_frame(pixels_all);
    if (self->state.allegro5)
        self->state.allegro5->add_frame(canvas_w, canvas_h, pixels_all);

    self->state.fps_counter->measure();
    self->send(renderer, streamer_ready::value);
    if (self->state.last_frame_streamed && *self->state.last_frame_streamed == frame_number) {
        if (self->state.ffmpeg)
            self->state.ffmpeg->finalize();
        if (self->state.ffmpeg_stream_flv)
            self->state.ffmpeg_stream_flv->finalize();
        if (self->state.ffmpeg_stream_hls)
            self->state.ffmpeg_stream_hls->finalize();
        if (self->state.allegro5)
            self->state.allegro5->finalize();
        aout(self) << "streamer completed frames: " << self->state.current_frame << ", with FPS: "
                   << (1000.0 / self->state.fps_counter->mean())
                   << " +/- " << self->state.fps_counter->stderr() << endl;
        self->quit(exit_reason::user_shutdown);
    }
    return true;
}


behavior streamer(stateful_actor<streamer_data>* self, std::optional<size_t> port) {
    if (port) publish_remote_actor("streamer", self, *port);

    // initialize fps counter
    self->state.fps_counter = std::make_shared<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
    self->state.fps_counter->setDescription("fps");
    self->state.fps_counter->startHistogramAtZero(true);
    return {
        [=](initialize, int render_window_at, string output_file, size_t bitrate, size_t fps, uint32_t settings, std::string stream_mode) {
            self->state.render_window_at = render_window_at;
            self->state.output_file = output_file;
            self->state.settings = settings;
            self->state.bitrate = bitrate;
            self->state.fps = fps;
            self->state.stream_mode = stream_mode;
        },
        [=](render_frame, struct data::job &job, data::pixel_data2 pixeldat, const caf::actor &renderer) {
            if (job.compress) {
                compress_vector<uint32_t> cv;
                cv.decompress(&pixeldat.pixels, job.width * job.height);
            }
            self->state.num_pixels = job.canvas_w * job.canvas_h;
            if (job.last_frame)
                self->state.last_frame_streamed = std::make_optional(job.frame_number);

            if (self->state.stream_mode == "hls") {
                if (!self->state.ffmpeg_stream_hls && bitset<32>(self->state.settings).test(0)) {
                    self->state.ffmpeg_stream_hls = make_shared<ffmpeg_hls_stream>(self->state.output_file,
                                                                                   self->state.bitrate,
                                                                                   self->state.fps,
                                                                                   job.canvas_w,
                                                                                   job.canvas_h);
                }
            }
            else if (self->state.stream_mode == "rtmp") {
                if (!self->state.ffmpeg_stream_flv && bitset<32>(self->state.settings).test(0)) {
                    self->state.ffmpeg_stream_flv = make_shared<ffmpeg_flv_stream>(self->state.output_file,
                                                                                   self->state.bitrate,
                                                                                   self->state.fps,
                                                                                   job.canvas_w,
                                                                                   job.canvas_h);
                }
            }
            else {
                if (!self->state.ffmpeg && bitset<32>(self->state.settings).test(0)) {
                    self->state.ffmpeg = make_shared<ffmpeg_h264_encode>(self->state.output_file,
                                                                         self->state.bitrate,
                                                                         self->state.fps,
                                                                         job.canvas_w,
                                                                         job.canvas_h);
                }
            }
            if (!self->state.allegro5 && bitset<32>(self->state.settings).test(1)) {
                self->state.allegro5 = make_unique<allegro5_window>(self->system(), self, self->state.render_window_at);
            }

            auto &rendered_jobs = self->state.rendered_jobs_set;
            rendered_jobs.emplace(job.frame_number, job.chunk, job.num_chunks, job.last_frame, pixeldat.pixels);
            while (process_buffer(self, renderer, self->state.current_frame, job))
                self->state.current_frame++;
        },
        [=](checkpoint) -> message {
            bool need_frames = self->mailbox().count() < self->state.min_items_in_streamer_queue;
            return make_message(need_frames);
        },
        [=](show_stats, string renderer_stats) {
            auto fps = (1000.0 / self->state.fps_counter->mean());
            aout(self) << "streamer[" << self->mailbox().count() << "] at frame: "
                       << self->state.current_frame << ", with FPS: " << fps
                       << " +/- " << self->state.fps_counter->stderr() << " ("
                       << ((self->state.num_pixels * sizeof(uint32_t) * fps) / 1024 / 1024) << " MiB/sec), "
                       << renderer_stats << endl;
        },
        [=](terminate_) {
            self->quit(exit_reason::user_shutdown);
        },
        [=](debug) {
            aout(self) << "streamer mailbox = " << self->mailbox().count() << /*" " << self->mailbox().counter() <<*/ endl;
        }
    };
}
