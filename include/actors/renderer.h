/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>
#include <chrono>
#include <data/pixels.hpp>
#include <mutex>

#include "common.h"

#include "data/job.hpp"
#include "rendering_engine_wrapper.h"

struct ALLEGRO_BITMAP;

struct worker_data {
  size_t worker_num = 0;
  ALLEGRO_BITMAP *bitmap = nullptr;
  rendering_engine_wrapper engine;
  uint32_t width = 0;
  uint32_t height = 0;
  size_t num_jobs_requested = 0;
  int64_t num_queue_per_worker = 1;
  bool is_remote_worker = false;
  std::optional<actor> renderer_ptr;
  std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double, std::milli>> previous_time =
      std::chrono::high_resolution_clock::now();
};

class MeasureInterval;

struct renderer_data {
  std::optional<caf::actor> streamer;
  std::optional<caf::actor> generator;
  std::vector<std::pair<std::string, int>> workers_vec;
  int64_t save_image = -1;
  bool realtime = false;
  std::set<data::job> job_queue;
  rendering_engine_wrapper engine;
  size_t job_sequence = 0;
  std::shared_ptr<MeasureInterval> jps_counter;
  std::set<std::pair<caf::actor, bool>> waiting_for_job;
  std::set<caf::actor> self_spawned_workers;
};

void fast_render_thread(caf::stateful_actor<worker_data> *self, bool output_each_frame);

// local worker knows who the renderer is already.. it was spawned by it.
behavior worker(caf::stateful_actor<worker_data> *self,
                caf::stateful_actor<renderer_data> *renderer,
                size_t worker_num,
                const int64_t &num_queue_per_worker);

behavior remote_worker(caf::stateful_actor<worker_data> *self,
                       size_t worker_num,
                       const std::string &renderer_host,
                       const int &renderer_port,
                       const int64_t &num_queue_per_worker);

behavior renderer(caf::stateful_actor<renderer_data> *self, std::optional<size_t> port);
