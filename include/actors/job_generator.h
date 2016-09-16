/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "common.h"

class MeasureInterval;

struct job_generator_data
{
    uint32_t canvas_w;
    uint32_t canvas_h;
    size_t current_job = 0;
    size_t num_chunks = 0;
    size_t lines_received = 0;
    size_t jobs_queued_for_renderer = 0;
    size_t max_jobs_queued_for_renderer = 1;
    bool has_max_jobs_queued_for_renderer = false;
    std::optional<actor> renderer_ptr;
    std::shared_ptr<MeasureInterval> fps_counter;
    size_t bitrate = 0;
    bool use_stdin = false;
};

behavior job_generator(stateful_actor<job_generator_data> *self, const std::string &filename, uint32_t canvas_w, uint32_t canvas_h, bool use_stdin = false, bool rendering_enabled = true, bool compress = false);

void call_print_exception(event_based_actor *self, std::string fn);
template <typename T> void call_print_exception(event_based_actor *self, std::string fn, T arg);
