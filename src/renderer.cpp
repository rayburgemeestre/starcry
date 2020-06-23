/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "actors/renderer.h"
#include "allegro5/allegro5.h"
#include "atom_types.h"
#include "benchmark.h"
#include "caf/io/all.hpp"
#include "common.h"
#include "data/job.hpp"
#include "data/pixels.hpp"
#include "util/assistant.h"
#include "util/compress_vector.h"
#include "util/remote_actors.h"

using namespace caf;

behavior create_worker_behavior(caf::stateful_actor<worker_data> *self,
                                const std::string &renderer_host,
                                const int &renderer_port,
                                const std::string &streamer_host,
                                const int &streamer_port,
                                bool output_each_frame = false) {
  connect_remote_worker(self->system(), "renderer", renderer_host, renderer_port, &self->state.renderer_ptr);
  connect_remote_worker(self->system(), "streamer", streamer_host, streamer_port, &self->state.streamer_ptr);
  self->state.previous_time = std::chrono::high_resolution_clock::now();
  return {[=](get_job, bool i_am_remote_worker, data::job job, const caf::actor &renderer, const caf::actor &streamer) {
            if (!i_am_remote_worker) {
              job.shapes = assistant->cache->retrieve(job);
            }
            self->state.job_queue.insert(job);
            self->send(self, process_job_v, i_am_remote_worker, renderer, streamer);
          },
          [=](process_job, bool i_am_remote_worker, const caf::actor &renderer, const caf::actor &streamer) {
            data::job job = *self->state.job_queue.cbegin();  // copy
            self->state.job_queue.erase(job);

            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> idle = current_time - self->state.previous_time;

            if (output_each_frame) {
              aout(self) << "processing[" << renderer_port << "]: frame " << job.frame_number << " chunk " << job.chunk
                         << " offsets " << job.offset_x << "," << job.offset_y << " worker " << self->state.worker_num
                         << " dimensions " << job.width << "x" << job.height << " mailbox=" << self->mailbox().count()
                         << " wasted idle = " << idle.count() << endl;
            }

            // make sure our bitmap is of the correct size.
            if ((self->state.width == 0 && self->state.height == 0) ||                   // not initialized
                (self->state.width != job.width || self->state.height != job.height) ||  // changed since previous
                self->state.bitmap == nullptr) {
              self->state.width = job.width;
              self->state.height = job.height;
              if (self->state.bitmap != nullptr) {
                al_destroy_bitmap(self->state.bitmap);
              }
              self->state.bitmap = al_create_bitmap(job.width, job.height);
            }

            // render + serialize + compress
            auto timer = TimerFactory::factory(TimerFactory::Type::BoostTimerImpl);
            timer->start();
            self->state.engine.render(self->state.bitmap,
                                      job.background_color,
                                      job.shapes,
                                      job.offset_x,
                                      job.offset_y,
                                      job.canvas_w,
                                      job.canvas_h,
                                      job.width,
                                      job.height,
                                      job.scale);
            if (job.save_image) {
              self->state.engine.write_image(self->state.bitmap, "output");
            }

            data::pixel_data2 dat;
            dat.job_number = job.job_number;
            dat.pixels = self->state.engine.serialize_bitmap2(self->state.bitmap, job.width, job.height);

            // compress the frame
            std::stringstream ss;
            if (job.compress) {
              compress_vector<uint32_t> cv;
              double compression_rate = 0;
              cv.compress(&dat.pixels, &compression_rate);
              if (output_each_frame) {
                ss << "compressed from 100% to " << compression_rate << "% ";
              }
            }
            // double render_time = timer->end();
            // ss << "mailbox = " << self->mailbox().count() << ", ";
            // ss << "render time = " << render_time << ".";
            // aout(self) << "worker idle, " << ss.str() << endl;

            auto &r = (self->state.renderer_ptr) ? *self->state.renderer_ptr : renderer;
            auto &s = (self->state.streamer_ptr) ? *self->state.streamer_ptr : streamer;

            // if the payload of the render frame etc., is high, I/O congestion may slow things down
            job.shapes = {};  // lighten the payload
            self->send<message_priority::high>(r, ready_v, self->state.worker_num, job);

            if (!i_am_remote_worker) {
              assistant->cache->take(dat);
            }
            self->send(s, render_frame_v, job, dat, renderer);

            self->state.previous_time = std::chrono::high_resolution_clock::now();
          }};
}

behavior remote_worker(caf::stateful_actor<worker_data> *self,
                       size_t worker_num,
                       const std::string &renderer_host,
                       const int &renderer_port,
                       const std::string &streamer_host,
                       const int &streamer_port) {
  self->state.worker_num = worker_num;
  rendering_engine_wrapper engine;
  engine.initialize();
  if (!publish_remote_actor("worker", static_cast<event_based_actor *>(self), worker_num)) {
    std::exit(1);
  }
  return create_worker_behavior(self, renderer_host, renderer_port, streamer_host, streamer_port, true);
}

behavior worker(caf::stateful_actor<worker_data> *self,
                const std::string &streamer_host,
                const int &streamer_port,
                size_t worker_num) {
  self->state.worker_num = worker_num;
  return create_worker_behavior(self, "", 0, streamer_host, streamer_port);
}

void send_jobs_to_streamer(caf::stateful_actor<renderer_data> *self) {
  for (size_t i = self->state.outstanding_jobs; i < self->state.max_outstanding_jobs; i++) {
    if (self->state.job_queue.empty()) {
      break;
    }
    auto job = *self->state.job_queue.cbegin();
    if (!self->state.workers_vec.empty()) {
      job.shapes = assistant->cache->retrieve(job);
    }
    bool is_remote_worker = !self->state.workers_vec.empty();
    self->send<message_priority::high>(
        *self->state.pool, get_job_v, is_remote_worker, job, self, *self->state.streamer);
    self->state.outstanding_jobs++;
    self->state.job_queue.erase(job);
  }
}

behavior renderer(caf::stateful_actor<renderer_data> *self, std::optional<size_t> port) {
  using workers_vec_type = std::vector<std::pair<std::string, int>>;
  publish_remote_actor("renderer", static_cast<event_based_actor *>(self), port ? *port : 0);
  // initialize jps counter
  self->state.jps_counter = std::make_shared<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
  self->state.jps_counter->setDescription("jps");
  self->state.jps_counter->startHistogramAtZero(true);
  return {
      [=](initialize,
          const caf::actor &streamer,
          const caf::actor &generator,
          const workers_vec_type &workers_vec,
          std::string streamer_host,
          int streamer_port,
          int64_t save_image) {
        self->state.remote_streamer_host = streamer_host;
        self->state.remote_streamer_port = streamer_port;
        self->state.streamer = streamer;
        self->state.generator = generator;
        self->state.workers_vec = workers_vec;
        self->state.save_image = save_image;
        self->state.engine.initialize();
        if (streamer_port) {
          connect_remote_worker(self->system(), "streamer", streamer_host, streamer_port, &self->state.streamer);
        }
      },
      [=](start, size_t num_workers) {
        aout(self) << "renderer started, num_workers = " << num_workers << endl;
        if (self->state.workers_vec.empty()) {
          auto worker_factory = [&]() -> actor {
            static size_t worker_num = 1000;
            aout(self) << "renderer spawning own worker" << endl;
            const auto &host = self->state.remote_streamer_host;
            const auto &port = self->state.remote_streamer_port;
            return self->spawn(worker, host, port, worker_num++);
          };
          auto tmp = actor_pool::make(self->context(), num_workers, worker_factory, actor_pool::round_robin());
          self->state.pool = std::make_unique<actor>(tmp);
        } else {
          aout(self) << "renderer started, num workers in text file = " << self->state.workers_vec.size() << endl;
          auto worker_factory = [&]() -> actor {
            static size_t index = 0;
            const auto &host = self->state.workers_vec[index].first;
            const auto &port = self->state.workers_vec[index].second;
            aout(self) << "renderer connecting to worker on : " << host << ":" << port << endl;
            std::optional<actor> actor_ptr;
            connect_remote_worker(self->system(), "worker", host, port, &actor_ptr);
            index++;
            return *actor_ptr;
          };
          auto num_workers = self->state.workers_vec.size();
          auto tmp = actor_pool::make(self->context(), num_workers, worker_factory, actor_pool::round_robin());
          self->state.pool = std::make_unique<actor>(tmp);
        }
        self->link_to(*self->state.pool);
      },
      [=](add_job, data::job job) {
        // render still image
        if (self->state.save_image == job.frame_number) {
          job.frame_number = 0;
          job.save_image = true;
          job.last_frame = true;
        } else if (self->state.save_image != -1) {
          self->send<message_priority::high>(*self->state.generator, job_processed_v);
          return;
        }
        self->state.job_queue.insert(job);
        send_jobs_to_streamer(self);
      },
      [=](ready, size_t worker_num, const data::job job) {
        self->send<message_priority::high>(*self->state.generator, job_processed_v);
        self->state.last_job_for_worker[worker_num] = job.job_number;
        self->state.job_sequence++;
      },
      [=](streamer_ready, size_t num_chunks) {
        self->state.outstanding_jobs -= num_chunks;
        send_jobs_to_streamer(self);
      },
      [=](show_stats) {
        std::stringstream ss;
        ss << "renderer[" << self->mailbox().count() << "] at job " << self->state.job_sequence << ", Q:";
        for (const auto p : self->state.last_job_for_worker) {
          const auto &job_number = p.second;
          ss << " " << job_number;
        }
        // aout(self) << "renderer at job: " << self->state.job_sequence << ", with jobs/sec: "
        //           << (1000.0 / self->state.jps_counter->mean())
        //           << " +/- " << self->state.jps_counter->stderr() << endl;
        self->send<message_priority::high>(*self->state.streamer, show_stats_v, ss.str());
      },
      [=](terminate_) {
        self->state.pool.release();
        self->quit(exit_reason::user_shutdown);
      },
      [=](debug) {
        aout(self) << "renderer mailbox = " << self->mailbox().count() /* << " " << self->mailbox().counter()*/ << endl;
      }};
}
