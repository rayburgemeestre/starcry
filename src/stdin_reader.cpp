/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "common.h"
#include "atom_types.h"
#include "actors/stdin_reader.h"

using namespace std;

behavior stdin_reader(stateful_actor<stdin_reader_data> *self, const caf::actor &job_generator) {
    self->link_to(job_generator);
    return {
        [=](start) {
            self->send(self, read_stdin_v);
        },
        [=](read_stdin) {
            string line;
            while (self->state.lines_send < self->state.max_lines_send){
                if (!getline(cin, line)) {
                    aout(self) << "stdin_reader: EOF" << endl;
                    self->send(job_generator, no_more_input_v);
                    return;
                }
                self->send(job_generator, input_line_v, line);
                self->state.lines_send++;
            }
            self->send(job_generator, checkpoint_v, self);
        },
        [=](checkpoint, size_t lines_received, bool paused_) {
            self->state.max_lines_send = lines_received + self->state.max_outgoing_lines;
            self->state.paused = paused_;
            if (self->state.paused) {
                self->delayed_send(job_generator, std::chrono::milliseconds(1), checkpoint_v, self);
            } else {
                // continue reading..
                self->delayed_send(self, std::chrono::milliseconds(1), read_stdin_v);
            }
        },
        [=](debug) {
            aout(self) << "stdin_reader mailbox = " << self->mailbox().count()
                       << /*" " << self->mailbox().counter() << */endl;
        },
    };
}
