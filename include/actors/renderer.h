/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "common.h"

#include "rendering_engine.hpp"
#include "data/job.hpp"

//class ALLEGRO_BITMAP;
struct worker_data
{
    size_t worker_num                   = 0;
    ALLEGRO_BITMAP * bitmap             = nullptr;
    rendering_engine engine; // TODO: convert to unique_ptr ?
    uint32_t width                      = 0;
    uint32_t height                     = 0;
    std::set<data::job> job_queue;
    size_t num_desired_messages_queued  = 20;
    size_t num_messaged_requested       = 0;
    size_t window_current               = 0;
    std::optional<actor> renderer_ptr;
    std::optional<actor> streamer_ptr;
};

class MeasureInterval;

struct renderer_data
{
    vector<caf::actor> workers;
    std::unique_ptr<actor> pool;
    std::optional<caf::actor> streamer;
    std::optional<caf::actor> generator;
    //const caf::actor *generator; // ok optional verneukt de zooi, inc num detached threads gets increased..
    vector<pair<string, int>> workers_vec;
    map<int, size_t> last_job_for_worker;
    size_t outstanding_jobs = 0;
    size_t max_outstanding_jobs = 20;
    std::set<data::job> job_queue;
    rendering_engine engine;

    // note these are only used by the renderer for local workers (worker).
    // spawned remote workers get this info from cli (remote_worker)
    string remote_streamer_host = "";
    int remote_streamer_port = 0;

    size_t job_sequence = 0;
    std::shared_ptr<MeasureInterval> jps_counter;
};

// local worker knows who the renderer is already.. it was spawned by it.
behavior worker(caf::stateful_actor<worker_data> * self, const string & streamer_host, const int &streamer_port, size_t worker_num);

behavior remote_worker(caf::stateful_actor<worker_data> * self, size_t worker_num, const string & renderer_host, const int &renderer_port, const std::string &streamer_host, const int &streamer_port);

behavior renderer(caf::stateful_actor<renderer_data> * self, std::optional<size_t> port);

