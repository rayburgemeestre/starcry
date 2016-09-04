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

#include "headers/codecfactory.h"
#include "headers/deltautil.h"

// public
using start                = atom_constant<atom("start     ")>;
using show_stats           = atom_constant<atom("show_stats")>;
using start_rendering      = atom_constant<atom("start_rend")>;
using stop_rendering       = atom_constant<atom("stop_rende")>;
using debug                = atom_constant<atom("debug     ")>;
using terminate_           = atom_constant<atom("terminate ")>;

// external
using get_job              = atom_constant<atom("get_job   ")>;
using del_job              = atom_constant<atom("del_job   ")>;
using need_frames          = atom_constant<atom("need_frame")>;

// internal
using ready                = atom_constant<atom("ready     ")>;
using render_frame         = atom_constant<atom("render_fra")>;
using process_job          = atom_constant<atom("process_jo")>;

size_t rendered_frame = 0;
size_t pool_outstanding = 0;
size_t expect_from_storage = 0;
size_t got_from_storage = 0;
size_t send_pool = 0;
size_t rcvd_pool = 0;

extern vector<uint32_t> compress_vector(vector<uint32_t> &in);
extern vector<uint32_t> decompress_vector(vector<uint32_t> &in);

using namespace FastPForLib;
IntegerCODEC &codec = *CODECFactory::getFromName("simdfastpfor256");

map<int, size_t> last_job_for_worker;

set<data::job> jobs_set;

template <typename T>
behavior create_worker_behavior(T self, bool output_each_frame = false) {
    return {
        [=](get_job, struct data::job job_in, const caf::actor &renderer) {
            jobs_set.insert(job_in);
            self->send(self, process_job::value, renderer);
        },
        [=](process_job, const caf::actor &renderer) {
            stringstream ss1;
            for (const auto &job_ : jobs_set) {
                ss1 << " " << job_.job_number;
            }
            cout << "jobs~~: " << ss1.str() << endl;

            auto jobmin = jobs_set.cbegin();
            data::job j = *jobmin; // copy
            jobs_set.erase(*jobmin);
            cout << "jobs~1: " << j.job_number << endl;

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
                aout(self) << "processing: frame " << j.frame_number << " chunk " << j.chunk << " offsets " << j.offset_x << "," << j.offset_y << " worker " << self->state.worker_num
                << " mailbox=" << self->mailbox().count() << " - " << self->mailbox().counter() << endl;
            } else {
#ifdef DEBUG
                ss << "frame " << j.frame_number << " chunk " << j.chunk << " offsets " << j.offset_x << "," << j.offset_y << " worker " << self->state.worker_num;
#endif
            }
            self->state.engine.render(self->state.bitmap, j.background_color, j.shapes, j.offset_x, j.offset_y, j.canvas_w, j.canvas_h, j.scale, ss.str());

            data::pixel_data2 dat;
            dat.pixels = self->state.engine.serialize_bitmap2(self->state.bitmap, j.width, j.height);

            // compression expirimental..
            if (j.compress) {
                using namespace FastPForLib;
                vector<uint32_t> compressed_output(dat.pixels.size() + 1024);
                size_t compressedsize = compressed_output.size();
                codec.encodeArray(dat.pixels.data(), dat.pixels.size(), compressed_output.data(), compressedsize);
                compressed_output.resize(compressedsize);
                compressed_output.shrink_to_fit();
                if (output_each_frame) {
                    aout(self) << "sending: frame compressed from 100\% to "
                    << 100.0 * static_cast<double>(compressed_output.size()) / static_cast<double>(dat.pixels.size())
                    << "%. mailbox still = " << self->mailbox().count() << " - " << self->mailbox().counter() <<
                    std::endl;
                }
                dat.pixels = compressed_output;
            }

            aout(self) << "IDLE." << endl;

            //dat.pixels = compress_vector(dat.pixels);
            //dat.pixels.shrink_to_fit();

            //self->send(renderer, ready::value, j, dat);
            //return make_message(ready::value, self->state.worker_num, j, dat);
            self->send(renderer, ready::value, self->state.worker_num, j, dat);
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

behavior renderer(event_based_actor* self, const caf::actor &job_storage, const caf::actor &streamer, const caf::actor &generator, const vector<pair<string, int>> &workers_vec, bool rendering_enabled, bool compress) {
    self->link_to(streamer);
    self->link_to(job_storage);
    /*if (!rendering_enabled) {
        self->link_to(generator);
    }*/
    rendering_engine engine;
    engine.initialize();

    counter.setDescription("fps");
    counter.startHistogramAtZero(true);
    return {
        [=](start, size_t num_workers) {
            num_workers_ = num_workers;
            aout(self) << "renderer started, num_workers = " << num_workers << endl;

            if (workers_vec.empty()) {
                auto worker_factory = [&]() -> actor {
                    static size_t worker_num = 1000;
                    aout(self) << "renderer spawning own worker" << endl;
                    return self->spawn(worker, worker_num++);
                };
                pool = std::move(std::make_unique<actor>(actor_pool::make(self->context(), num_workers, worker_factory, actor_pool::round_robin())));
                self->link_to(*pool);
                for (size_t i=0; i<num_workers_; i++) self->send(self, render_frame::value, static_cast<size_t>(0));
            }
            else {
                aout(self) << "renderer started, num workers in text file = " << workers_vec.size() << endl;
                num_workers_ = workers_vec.size();
                auto worker_factory = [&]() -> actor {
                    static size_t index = 0;
                    aout(self) << "renderer connecting to worker on : " << workers_vec[index].first << ":" << workers_vec[index].second << endl;
                    auto p = self->system().middleman().remote_actor(workers_vec[index].first, workers_vec[index].second);
                    if (!p) {
                        aout(self) << "spawning remote actor failed: " << self->system().render(p.error()) << endl;
                    }
                    self->monitor(*p); // so we can notice if the connection closes.
                    self->set_down_handler([=](down_msg& dm) {
                        cout << "TODO: ACTOR FROM POOL DOWN!!" << endl;
                    });
                    index++;
                    return *p;
                };
                pool = std::move(std::make_unique<actor>(actor_pool::make(self->context(), num_workers, worker_factory, actor_pool::round_robin())));
                self->link_to(*pool);
            }
            self->send<message_priority::high>(self, render_frame::value, static_cast<size_t>(0));
        },
        [=](start_rendering) {
            rendering_active_ = true;
        },
        [=](stop_rendering) {
            rendering_active_ = false;
        },
        [=](render_frame, size_t next_frame) {
//            if (!rendering_active_) {
//                std::this_thread::sleep_for(std::chrono::milliseconds(100));
//                // TODO: caf015
//                // TODO: also replace by scoped_actor ?
//                self->request(streamer, infinite, need_frames::value).then(
//                    [=](need_frames, bool answer) {
//                        if (answer) {
//                            rendering_active_ = true;
//                        }
//                    }
//                );
//                self->send(self, render_frame::value, next_frame);
//                return;
//            }

            // TODO: want to convert this to asynchronous, but this gave me segfaults..
            //  Need to figure out why that is..
            //scoped_actor s{self->system()};
            while (pool_outstanding < (num_workers_ * 20)) {
                cout << "renderer requesting A NEW job: " << next_frame << endl;
                self->send(job_storage, get_job::value, self);
                expect_from_storage++;
                pool_outstanding++;
            }
        },

                [=](get_job, bool false_) {
                    cout << "receiving: nothing!" << endl;
                    //self->send(*pool, get_job::value, j);
                    //rendered_frame++;
                    got_from_storage++;
                    pool_outstanding--; // there was actually nothing prepared yet
                    //if (rendered_frame == size_t(0))
                    {
                        self->send(job_storage, get_job::value, self);
                        pool_outstanding++;
                        expect_from_storage++;
                    }
                    //self->send<message_priority::high>(self, render_frame::value, rendered_frame); // this may be a wrong choice..
                },
                [=](get_job, data::job j) {
                    got_from_storage++;
                    cout << "receiving: " << j.job_number << " while pool has queue of: " << pool_outstanding << " p:" << send_pool << " / " << expect_from_storage << "/" << got_from_storage << endl;
                    j.compress = compress;
                    self->send<message_priority::high>(*pool, get_job::value, j, self);
                    send_pool++;
                    rendered_frame++;
                    //self->send<message_priority::high>(self, render_frame::value, rendered_frame);
                    //pool_outstanding++;
                },

        [=](ready, size_t worker_num, struct data::job j, data::pixel_data2 pixeldat) {
            rcvd_pool++;

            last_job_for_worker[worker_num] = j.job_number;
            //pixeldat.pixels = decompress_vector(pixeldat.pixels);
            if (j.compress) {
                // experimental decompression
                std::vector<uint32_t> mydataback(j.width * j.height);
                size_t recoveredsize = mydataback.size();
                codec.decodeArray(pixeldat.pixels.data(), pixeldat.pixels.size(), mydataback.data(), recoveredsize);
                mydataback.resize(recoveredsize);
                pixeldat.pixels = mydataback;
            }

            cout << "REady enzo" << " " << rcvd_pool << "/" << send_pool << endl;
            pool_outstanding--;
            //while (pool_outstanding < (num_workers_ * 4)) {
                cout << "renderer requesting A NEW job" << endl;
                self->send(job_storage, get_job::value, self);
                pool_outstanding++;
                expect_from_storage++;
            //}

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
//            self->send<message_priority::high>(self, render_frame::value, rendered_frame);
        },
        [=](show_stats) {
            stringstream ss;
            ss << "renderer[" << self->mailbox().count() << "],";
            for (const auto p : last_job_for_worker)
            {
                const auto &worker_num = p.first;
                const auto &job_number = p.second;
                ss << " " << job_number;// << (job ? to_string(*job) : "x");
            }
            ss << " pool_outstanding = " << pool_outstanding;
            aout(self) << "renderer at job: " << job_sequence << ", with jobs/sec: " << (1000.0 / counter.mean())
                       << " +/- " << counter.stderr() << endl;
            self->send<message_priority::high>(streamer, show_stats::value, ss.str());
        },
        [=](debug) {
            aout(self) << "renderer mailbox = " << self->mailbox().count() << " " << self->mailbox().counter() << endl;
        },
        [=](terminate_) {
            aout(self) << "terminating.." << endl;
            self->quit(exit_reason::user_shutdown);
        }
    };
}

