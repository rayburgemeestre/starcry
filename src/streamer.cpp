/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "actors/streamer.h"

#include "benchmark.h"
#include <bitset>
#include "data/job.hpp"
#include "data/pixels.hpp"
#include "output_action.hpp"
#include "util/compress_vector.h"
#include "caf/io/all.hpp"
#include "util/remote_actors.h"


using terminate_           = atom_constant<atom("terminate ")>;
// public
using start            = atom_constant<atom("start     ")>;
using render_frame     = atom_constant<atom("render_fra")>;
using show_stats       = atom_constant<atom("show_stats")>;
using need_frames      = atom_constant<atom("need_frame")>;
using debug            = atom_constant<atom("debug     ")>;
using initialize       = atom_constant<atom("initialize")>;
using checkpoint       = atom_constant<atom("checkpoint")>;

// external
using del_job          = atom_constant<atom("del_job   ")>;
using stop_rendering   = atom_constant<atom("stop_rende")>;
using streamer_ready   = atom_constant<atom("streamer_r")>;

// internal
using ready            = atom_constant<atom("ready     ")>;
using recheck_test     = atom_constant<atom("recheck_te")>;

auto benchmark_class2 = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter2 = static_cast<MeasureInterval &>(*benchmark_class2.get());

struct rendered_job
{
    size_t frame_number;
    size_t chunk;
    size_t num_chunks;
    bool last_frame;
    std::vector<uint32_t> pixels;

    rendered_job(size_t frame_number,
                 size_t chunk,
                 size_t num_chunks,
                 bool last_frame,
                 std::vector<uint32_t> &pixels)
    : frame_number(frame_number),
      chunk(chunk),
      num_chunks(num_chunks),
      last_frame(last_frame),
      pixels(pixels)
    {}

    bool operator <(const rendered_job &other) const {
        if (frame_number == other.frame_number)
            return chunk < other.chunk;
        return frame_number < other.frame_number;
    }
};

using namespace std;
//vector<tuple<size_t, size_t, size_t, bool, vector<uint32_t>>> fake_buffer, matches;
set<rendered_job> rendered_jobs_set;
std::optional<size_t> last_frame_streamed;

// TODO; add state to streamer object
// TODO; refactor this to regular objects.. this doesn't make sense..
// as a temporary workaround putting stuff inside a unique_ptr...
unique_ptr<output_action<ffmpeg_h264_encode>> ffmpeg;
unique_ptr<output_action<allegro5_window>> allegro5;
unique_ptr<output_aggregate> outputs;

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

bool process_buffer(stateful_actor<streamer_data> *self, const actor &renderer, size_t frame_number, size_t num_chunks, uint32_t canvas_w, uint32_t canvas_h) {
    if (!all_frame_chunks_present(rendered_jobs_set, frame_number, num_chunks))
        return false;
    // we split the image horizontally so we can just concat all pixels here
    vector<uint32_t> pixels_all;
    pixels_all.reserve(canvas_w * canvas_h);
    for (size_t i=0; i<num_chunks; i++) {
        pixels_all.insert(pixels_all.end(), rendered_jobs_set.cbegin()->pixels.cbegin(), rendered_jobs_set.cbegin()->pixels.cend() );
        rendered_jobs_set.erase(rendered_jobs_set.begin());
    }
    //cout << "streamer completed frame: " << self->state.current_frame << " number of pixels equal to: "
    //     << pixels_all.size() << endl; // debug
    outputs->add_frame(canvas_w, canvas_h, pixels_all);
    counter2.measure();
    self->send(renderer, streamer_ready::value);
    if (last_frame_streamed && *last_frame_streamed == frame_number) {
        outputs->finalize();
        aout(self) << "streamer completed frames: " << self->state.current_frame << ", with FPS: " << (1000.0 / counter2.mean())
                   << " +/- " << counter2.stderr() << endl;
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
        [=](initialize, int render_window_at, string output_file, size_t bitrate, uint32_t settings) {
            self->state.render_window_at = render_window_at;
            self->state.output_file = output_file;
            self->state.settings = settings;

            ffmpeg   = std::make_unique<output_action<ffmpeg_h264_encode>>();
            allegro5 = std::make_unique<output_action<allegro5_window>>(self->system());
            outputs  = std::make_unique<output_aggregate>(*ffmpeg.get(), *allegro5.get());
            if (bitset<32>(settings).test(0)) {
                ffmpeg->enable(true);
                ffmpeg->set_filename(output_file);
                ffmpeg->set_bitrate(bitrate);
            }
            if (bitset<32>(settings).test(1)) {
                allegro5->enable(true);
            }
        },
        [=](render_frame, struct data::job &job, data::pixel_data2 pixeldat, const caf::actor &renderer) {
            //cout << "streamer processing... " << job.job_number << endl;
            if (job.compress) {
                compress_vector<uint32_t> cv;
                cv.decompress(&pixeldat.pixels, job.width * job.height);
            }

            vector<uint32_t> &pixels = pixeldat.pixels;

            self->state.num_pixels = job.canvas_w * job.canvas_h;
            if (job.last_frame)
                last_frame_streamed = std::make_optional(job.frame_number);

            outputs->initialize(job.canvas_w, job.canvas_h, self, self->state.render_window_at);

            rendered_jobs_set.emplace(job.frame_number, job.chunk, job.num_chunks, job.last_frame, pixels);
            while (process_buffer(self, renderer, self->state.current_frame, job.num_chunks, job.canvas_w, job.canvas_h))
                self->state.current_frame++;

            //cout << "IDLE." << endl;
        },
        [=](checkpoint) -> message {
            bool need_frames = self->mailbox().count() < self->state.min_items_in_streamer_queue;
            return make_message(need_frames);
        },
        [=](show_stats, string renderer_stats) {
            auto fps = (1000.0 / counter2.mean());
            aout(self) << "streamer[" << self->mailbox().count() << "] at frame: " << self->state.current_frame << ", with FPS: " << fps
                       << " +/- " << counter2.stderr() << " (" << ((self->state.num_pixels * sizeof(uint32_t) * fps) / 1024 / 1024) << " MiB/sec), "
                       << renderer_stats << endl;
        },
        [=](debug) {
            aout(self) << "streamer mailbox = " << self->mailbox().count() << " " << self->mailbox().counter() << endl;
        },
        [=](terminate_) {
            aout(self) << "terminating.." << endl;
            self->quit(exit_reason::user_shutdown);
        }
    };
}
