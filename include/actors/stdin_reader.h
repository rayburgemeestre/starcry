/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "common.h"

struct stdin_reader_data
{
    int max_num_lines_batch = 500;
    int desired_max_mailbox_generator = 100;
    size_t lines_send = 0;
    size_t max_outgoing_lines = 100;
    size_t max_lines_send = max_outgoing_lines;
    bool paused = false;
};

/**
 * Standard input reader that reads a max. of `max_num_lines_batch` lines a time.
 *
 * If the max. number of lines has been read, or there is no input coming from stdin,
 *  it will ask the job_generator for a checkpoint.
 * The job_generator responds to the checkpoint with an update: how many lines it has
 *  received from the stdin_reader and whether stdin_reader should be paused.
 * When paused it will request a new checkpoint, over and over, until it is unpaused,
 *  then it will continue reading stdin.
 */
behavior stdin_reader(stateful_actor<stdin_reader_data> *self, const caf::actor &job_generator);
