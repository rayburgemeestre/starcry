/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>

#include "common.h"
#include "data/job.hpp"
#include "data/pixel_data.hpp"
#include "data/pixels.hpp"

// clang-format off
CAF_BEGIN_TYPE_ID_BLOCK(starcry, first_custom_type_id)

  CAF_ADD_TYPE_ID(starcry, (std::vector<uint32_t>))
  CAF_ADD_TYPE_ID(starcry, (std::vector<std::pair<std::string, int>>))
  CAF_ADD_TYPE_ID(starcry, (data::job))
  CAF_ADD_TYPE_ID(starcry, (data::pixel_data))
  CAF_ADD_TYPE_ID(starcry, (data::pixel_data2))

  CAF_ADD_ATOM(starcry, add_job)
  CAF_ADD_ATOM(starcry, checkpoint)
  CAF_ADD_ATOM(starcry, debug)
  CAF_ADD_ATOM(starcry, del_job)
  CAF_ADD_ATOM(starcry, get_job)
  CAF_ADD_ATOM(starcry, initialize)
  CAF_ADD_ATOM(starcry, input_line)
  CAF_ADD_ATOM(starcry, job_processed)
  CAF_ADD_ATOM(starcry, need_frames)
  CAF_ADD_ATOM(starcry, next_frame)
  CAF_ADD_ATOM(starcry, no_more_input)
  CAF_ADD_ATOM(starcry, output_line)
  CAF_ADD_ATOM(starcry, process_job)
  CAF_ADD_ATOM(starcry, read_stdin)
  CAF_ADD_ATOM(starcry, ready)
  CAF_ADD_ATOM(starcry, recheck_test)
  CAF_ADD_ATOM(starcry, render)
  CAF_ADD_ATOM(starcry, render_frame)
  CAF_ADD_ATOM(starcry, show_stats)
  CAF_ADD_ATOM(starcry, start)
  CAF_ADD_ATOM(starcry, stop_rendering)
  CAF_ADD_ATOM(starcry, streamer_ready)
  CAF_ADD_ATOM(starcry, terminate_)
  CAF_ADD_ATOM(starcry, write_frame)

CAF_END_TYPE_ID_BLOCK(starcry)
// clang-format on
