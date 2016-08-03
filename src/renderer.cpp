/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "actors/renderer.h"
#include "data/job.hpp"
#include "data/pixels.hpp"

#include "benchmark.h"

#include "caf/io/all.hpp"

// public
using start                = atom_constant<atom("start     ")>;
using show_stats           = atom_constant<atom("show_stats")>;
using start_rendering      = atom_constant<atom("start_rend")>;
using stop_rendering       = atom_constant<atom("stop_rende")>;
using debug                = atom_constant<atom("debug     ")>;

// external
using get_job              = atom_constant<atom("get_job   ")>;
using del_job              = atom_constant<atom("del_job   ")>;
using need_frames          = atom_constant<atom("need_frame")>;

// internal
using ready                = atom_constant<atom("ready     ")>;
using render_frame         = atom_constant<atom("render_fra")>;

size_t rendered_frame = 0;

template <typename T>
behavior create_worker_behavior(T self, bool output_each_frame = false) {
    return {
        [=](get_job, struct data::job j) -> message {
            // make sure our bitmap is of the correct size.
            if ((self->state.width == 0 && self->state.height == 0) || // not initialized
                (self->state.width != j.width || self->state.height != j.height) || // changed since previous
                self->state.bitmap == nullptr
            ){
                self->state.width = j.width;
                self->state.height = j.height;
                if (self->state.bitmap != nullptr) {
                    al_destroy_bitmap(self->state.bitmap);
                }
                self->state.bitmap = al_create_bitmap(j.width, j.height);
            }

            // render
            stringstream ss;
            if (output_each_frame) {
                aout(self) << "frame " << j.frame_number << " chunk " << j.chunk << " offsets " << j.offset_x << "," << j.offset_y << " worker " << self->state.worker_num << endl;
            } else {
#ifdef DEBUG
                ss << "frame " << j.frame_number << " chunk " << j.chunk << " offsets " << j.offset_x << "," << j.offset_y << " worker " << self->state.worker_num;
#endif
            }
            self->state.engine.render(self->state.bitmap, j.background_color, j.shapes, j.offset_x, j.offset_y, j.canvas_w, j.canvas_h, j.scale, ss.str());

            data::pixel_data2 dat;
            dat.pixels = self->state.engine.serialize_bitmap2(self->state.bitmap, j.width, j.height);

            //self->send(renderer, ready::value, j, dat);
            return make_message(ready::value, j, dat);
        }
    };
}

behavior remote_worker(caf::stateful_actor<worker_data> * self, size_t worker_num) {
    self->state.worker_num = worker_num;
    rendering_engine engine;
    engine.initialize();
    aout(self) << "worker publishing myself on port: " << worker_num << endl;
    auto p = self->system().middleman().publish(static_cast<actor>(self), worker_num, nullptr, true);
    if (!p) {
        aout(self) << "worker publishing FAILED..: " << self->system().render(p.error()) << endl;
    }
    else if (*p != worker_num) {
        aout(self) << "worker publishing FAILED.." << endl;
    }
    return create_worker_behavior(self, true);
}

behavior worker(caf::stateful_actor<worker_data> * self, size_t worker_num) {
    self->state.worker_num = worker_num;
    return create_worker_behavior(self);
}

// move to class data
std::vector<data::job> jobs_done;
size_t job_sequence = 0;
std::unique_ptr<actor> pool;

auto benchmark_class = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter = static_cast<MeasureInterval &>(*benchmark_class.get());

//struct renderer_data
//{
//    rendering_engine engine;
//};

map<size_t, vector<uint32_t>> pixel_store;
// TODO: need class data...
bool rendering_active_ = true;
size_t num_workers_ = 0;

std::optional<size_t> empty_job_slot;
vector<std::optional<size_t>> render_worker_job_in_progress;
vector<actor> render_workers;

behavior renderer(event_based_actor* self, const caf::actor &job_storage, const caf::actor &streamer, const vector<pair<string, int>> &workers_vec) {
    self->link_to(streamer);
    self->link_to(job_storage);
    rendering_engine engine;
    engine.initialize();

    counter.setDescription("fps");
    counter.startHistogramAtZero(true);
    return {
        [=](start, size_t num_workers) {
            num_workers_ = num_workers;
            aout(self) << "renderer started, num_workers = " << num_workers << endl;

            if (workers_vec.empty()) {
                render_workers.reserve(num_workers);
                render_worker_job_in_progress.reserve(num_workers);
                for (size_t worker_num=0; worker_num<num_workers; worker_num++) {
                    aout(self) << "renderer spawning local worker " << worker_num << endl;
                    render_workers.push_back(self->spawn(worker, worker_num));
                    render_worker_job_in_progress.push_back(empty_job_slot);
                    self->link_to(render_workers[worker_num]);
                }
            }
            else {
                render_workers.reserve(num_workers);
                render_worker_job_in_progress.reserve(num_workers);
                size_t index = 0;
                for (const auto &p : workers_vec) {
                    const auto &host = p.first;
                    const auto &port = p.second;
                    auto remoteactor = self->system().middleman().remote_actor(host, port);
                    if (!remoteactor) {
                        aout(self) << "spawning remote actor failed: " << self->system().render(remoteactor.error()) << endl;
                    }
                    render_workers.push_back(*remoteactor);
                    render_worker_job_in_progress.push_back(empty_job_slot);
                    index++;
                }
            }
            self->send(self, render_frame::value);
        },
        [=](start_rendering) {
            rendering_active_ = true;
        },
        [=](stop_rendering) {
            rendering_active_ = false;
        },
        [=](render_frame) {
            if (!rendering_active_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                // TODO: caf015
                // TODO: also replace by scoped_actor ?
                self->request(streamer, infinite, need_frames::value).then(
                    [=](need_frames, bool answer) {
                        if (answer) {
                            rendering_active_ = true;
                        }
                    }
                );
                self->send(self, render_frame::value);
                return;
            }

            size_t worker_index = 0;
            for (const auto &job_number : render_worker_job_in_progress) {
                if (!job_number) {
                    scoped_actor s{self->system()};
                    s->request(job_storage, infinite, get_job::value, rendered_frame).receive(
                        [=](data::job j) {
                            self->send(render_workers[worker_index], get_job::value, j);
                            render_worker_job_in_progress[worker_index] = j.job_number;
                            rendered_frame++;
                        },
                        [=](error &e) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            self->send(self, render_frame::value);
                        }
                    );
                }
                worker_index++;
            }
        },
        [=](ready, struct data::job j, data::pixel_data2 pixeldat) {
            pixel_store[j.job_number] = pixeldat.pixels;
            auto send_to_streamer = [&](struct data::job &job) {
                counter.measure();
                self->send(streamer, render_frame::value, job, pixel_store[job.job_number], self);
                auto it = pixel_store.find(job.job_number);
                pixel_store.erase(it);
            };
            if (j.job_number == job_sequence) {
                send_to_streamer(j);
                job_sequence++;
                while (true) {
                    auto pos = find_if(jobs_done.begin(), jobs_done.end(), [&](auto &job) {
                        return job.job_number == job_sequence;
                    });
                    if (pos == jobs_done.end()) {
                        break;
                    }
                    send_to_streamer(*pos);
                    jobs_done.erase(pos);
                    job_sequence++;
                }
            } else {
                jobs_done.push_back(j);
            }

            for (auto &job_number : render_worker_job_in_progress) {
                if (*job_number == j.job_number) {
                    job_number = empty_job_slot;
                    break;
                }
            }
            self->send(self, render_frame::value);
        },
        [=](show_stats) {
            aout(self) << "renderer at job: " << job_sequence << ", with jobs/sec: " << (1000.0 / counter.mean())
                       << " +/- " << counter.stderr() << endl;
        },
        [=](debug) {
            stringstream ss;
            for (const auto job : render_worker_job_in_progress)
                ss << ", " << (job ? to_string(*job) : "x");
            aout(self) << "renderer mailbox = " << self->mailbox().count() << " --> " << ss.str() << endl;
        }
    };
}

