/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <thread>

#include "actors/renderer.h"
#include "allegro5/allegro5.h"
#include "atom_types.h"
#include "benchmark.h"
#include "caf/io/all.hpp"
#include "common.h"
#include "data/job.hpp"
#include "data/pixels.hpp"
#include "render_threads.h"
#include "util/assistant.h"
#include "util/compress_vector.h"
#include "util/remote_actors.h"

render_threads threads;

behavior renderer(caf::stateful_actor<renderer_data> *self, std::optional<size_t> port) {
  publish_remote_actor("renderer", static_cast<event_based_actor *>(self), port ? *port : 0);
  // initialize jps counter
  self->state.jps_counter = std::make_shared<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
  self->state.jps_counter->setDescription("jps");
  self->state.jps_counter->startHistogramAtZero(true);
  return {
      [=](initialize,
          const caf::actor &streamer,
          const caf::actor &generator,
          int64_t save_image,
          bool realtime,
          bool to_files) {
        self->state.streamer = streamer;
        self->state.generator = generator;
        self->state.save_image = save_image;
        self->state.realtime = realtime;
        self->state.to_files = to_files;
        self->state.engine.initialize();
      },
      [=](start, size_t num_workers, int64_t num_queue_per_worker) {
        self->state.num_queue_per_worker = num_queue_per_worker;
        for (int worker_num = 0; worker_num < num_workers; worker_num++) {
          auto w = self->system().spawn(worker, self, size_t(worker_num), num_queue_per_worker);
          self->link_to(w);
          self->send(w, start_v);

          // for some reason, the fact that the worker is linked does not result in it tearing down on user_shutdown
          self->state.self_spawned_workers.insert(w);
        }
      },
      [=](register_worker, caf::actor &sender, bool is_remote_worker) {
        // TODO: add worker to allow list or something
        self->send(sender, register_worker_ok_v, self->state.num_queue_per_worker);
      },
      [=](add_job, data::job job) {
        // // render still image
        // if (self->state.save_image == job.frame_number) {
        //   job.frame_number = 0;
        //   job.save_image = true;
        //   job.last_frame = true;
        // } else if (self->state.save_image != -1) {
        //   self->send(*self->state.generator, job_processed_v);
        //   return;
        // }
        if (!self->state.waiting_for_job.empty()) {
          // forward to a waiting actor immediately
          auto lucky_actor = self->state.waiting_for_job.back();
          self->state.waiting_for_job.pop_back();
          // unpack for remote worker
          if (lucky_actor.second) {
            job.shapes = assistant->cache->retrieve(job);
          }
          self->send<message_priority::high>(lucky_actor.first,
                                             get_job_v,
                                             job,
                                             self,
                                             *self->state.streamer,
                                             self->state.to_files,
                                             (int64_t)std::time(0));
        } else {
          self->state.job_queue.insert(job);
        }
      },
      [=](pull_job, caf::actor &sender, bool is_remote_worker, int64_t debug) {
        int64_t debug2 = std::time(0);
        // std::cout << "This pull job request was in transit for: " << (debug2 - debug) << std::endl;
        if (self->state.realtime) {
          // streamer is in charge of requesting frames
        } else {
          // request new frame immediately
          self->send<message_priority::high>(*self->state.generator, job_processed_v);
        }
        if (self->state.job_queue.empty()) {
          self->state.waiting_for_job.emplace_back(std::make_pair(sender, is_remote_worker));
          return;
        }
        auto job = *self->state.job_queue.cbegin();
        if (is_remote_worker) {
          job.shapes = assistant->cache->retrieve(job);
        }
        self->state.job_queue.erase(job);
        self->send<message_priority::high>(
            sender, get_job_v, job, self, *self->state.streamer, self->state.to_files, (int64_t)std::time(0));
      },
      [=](render_frame, data::job job, data::pixel_data2 dat, bool is_remote_worker) {
        if (is_remote_worker) {
          assistant->cache->take(job);
          assistant->cache->take(dat);
        }
        self->send(*self->state.streamer, render_frame_v, job, dat, self);
      },
      [=](streamer_ready, size_t num_chunks) {
        if (self->state.realtime) {
          self->send<message_priority::high>(*self->state.generator, job_processed_v);
        } else {
          // TODO: streamer can pause / unpause in case of non-realtime rendering
        }
      },
      [=](show_stats) {
        std::stringstream ss;
        ss << "renderer[" << self->mailbox().count() << "] at job " << self->state.job_sequence
           << ", Q: <TODO: re-implement>";
        self->send(*self->state.streamer, show_stats_v, ss.str());
      },
      [=](terminate_) {
        std::cout << "user called terminate on me" << std::endl;
        for (const auto &worker : self->state.self_spawned_workers) {
          self->send(worker, terminate__v);
        }
        self->quit(exit_reason::user_shutdown);
      },
      [=](debug) {
        aout(self) << "renderer mailbox = " << self->mailbox().count() /* << " " << self->mailbox().counter()*/ << endl;
      }};
}

void fast_render_thread(caf::stateful_actor<worker_data> *self, bool output_each_frame) {
  data::job job;
  while (threads.keep_running(self->state.worker_num)) {
    {
      const auto items = threads.num_queued(self->state.worker_num);
      if (items == 0) {
        continue;  // nothing to do
      }
      data::job job;
      bool to_file = false;
      std::tie(job, to_file) = threads.job(self->state.worker_num);

      auto current_time = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> idle = current_time - self->state.previous_time;

      if (output_each_frame) {
        std::cout << "worker=" << self->state.worker_num << " wasted=" << idle.count() << " items=" << items
                  << std::endl;
      }

      // make sure our bitmap is of the correct size.
      if ((self->state.width == 0 && self->state.height == 0) ||  // not initialized
          (self->state.width != job.width || self->state.height != job.height) ||
          // changed since previous
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
      if (!self->state.is_remote_worker) {
        job.shapes = assistant->cache->retrieve(job);
      }
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

      if (to_file) {
        char buf[512] = {0x00};
        sprintf(buf, "frame_job_%05ld", dat.job_number);
        self->state.engine.write_image(self->state.bitmap, buf);
      } else {
        dat.pixels = self->state.engine.serialize_bitmap2(self->state.bitmap, job.width, job.height);

        // compress the frame
        // std::stringstream ss;
        if (job.compress) {
          compress_vector<uint32_t> cv;
          double compression_rate = 0;
          cv.compress(&dat.pixels, &compression_rate);
          // if (output_each_frame) {
          //    ss << "compressed from 100% to " << compression_rate << "% ";
          // }
        }

        if (!self->state.is_remote_worker) {
          assistant->cache->take(dat);
        }
      }

      threads.add_result(self->state.worker_num, job, dat);

      self->state.previous_time = std::chrono::high_resolution_clock::now();
    }
  }
}

behavior create_worker_behavior(caf::stateful_actor<worker_data> *self,
                                const int64_t &num_queue_per_worker,
                                bool output_each_frame,
                                bool i_am_remote_worker) {
  self->state.previous_time = std::chrono::high_resolution_clock::now();
  self->state.num_queue_per_worker = num_queue_per_worker;
  self->state.is_remote_worker = i_am_remote_worker;

  return {
      [=](start) { self->send(*self->state.renderer_ptr, register_worker_v, self, self->state.is_remote_worker); },
      [=](register_worker_ok, int64_t num_queue_per_worker_server_side) {
        self->state.num_queue_per_worker = num_queue_per_worker_server_side;
        threads.run(self->state.worker_num,
                    std::make_unique<std::thread>([&]() { fast_render_thread(self, output_each_frame); }));
        self->send(self, pull_job_v);
      },
      [=](pull_job) {
        size_t need = self->state.num_queue_per_worker - threads.num_queued(self->state.worker_num);
        need -= self->state.num_jobs_requested;

        // send all frames that are done
        threads.for_each_and_clear(self->state.worker_num, [&](const data::job &job, const data::pixel_data2 &dat) {
          self->send<message_priority::high>(
              *self->state.renderer_ptr, render_frame_v, job, dat, self->state.is_remote_worker);
        });

        // request new frame(s)
        for (int i = 0; i < need; i++) {
          self->send<message_priority::high>(
              *self->state.renderer_ptr, pull_job_v, self, self->state.is_remote_worker, (int64_t)std::time(0));
          self->state.num_jobs_requested++;
        }

        // loop
        self->delayed_send(self, std::chrono::milliseconds(1), pull_job_v);

        // below can be too agressive (generator cannot get through to renderer with new frames)
        // self->send(self, pull_job_v);
      },
      [=](get_job,
          data::job job,
          const caf::actor &renderer,
          const caf::actor &streamer,
          bool to_files,
          int64_t debug) {
        // int64_t debug2 = std::time(0);
        // std::cout << "This job was in transit for: " << (debug2 - debug) << std::endl;
        if (self->state.num_jobs_requested > 0) self->state.num_jobs_requested--;
        threads.add_job(self->state.worker_num, job, to_files);
      },
      [=](terminate_) {
        threads.shutdown();
        self->quit(exit_reason::user_shutdown);
      },
  };
}

behavior remote_worker(caf::stateful_actor<worker_data> *self,
                       size_t worker_num,
                       const std::string &renderer_host,
                       const int &renderer_port,
                       const int64_t &num_queue_per_worker) {
  self->state.worker_num = worker_num;
  bool ret = connect_remote_worker(self->system(), "renderer", renderer_host, renderer_port, &self->state.renderer_ptr);
  rendering_engine_wrapper engine;
  engine.initialize();
  return create_worker_behavior(self, num_queue_per_worker, true, true);
}

behavior worker(caf::stateful_actor<worker_data> *self,
                caf::stateful_actor<renderer_data> *renderer,
                size_t worker_num,
                const int64_t &num_queue_per_worker) {
  self->state.worker_num = worker_num;
  self->state.renderer_ptr = static_cast<caf::actor>(renderer);
  return create_worker_behavior(self, num_queue_per_worker, false, false);
}
