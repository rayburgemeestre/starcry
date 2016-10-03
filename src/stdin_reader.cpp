/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "actors/stdin_reader.h"

using start            = atom_constant<atom("start     ")>;
using input_line       = atom_constant<atom("input_line")>;
using read_stdin       = atom_constant<atom("read_stdin")>;
using no_more_input    = atom_constant<atom("no_more_in")>;
using debug            = atom_constant<atom("debug     ")>;
using checkpoint       = atom_constant<atom("checkpoint")>;

using namespace std;

behavior stdin_reader(stateful_actor<stdin_reader_data> *self, const caf::actor &job_generator) {
    self->link_to(job_generator);
    return {
        [=](start) {
            self->send(self, read_stdin::value);
        },
        [=](read_stdin) {
            string line;
            while (self->state.lines_send < self->state.max_lines_send){
                if (!getline(cin, line)) {
                    aout(self) << "stdin_reader: EOF" << endl;
                    self->send(job_generator, no_more_input::value);
                    return;
                }
                self->send(job_generator, input_line::value, line);
                self->state.lines_send++;
            }
            self->send(job_generator, checkpoint::value, self);
        },
        [=](checkpoint, size_t lines_received, bool paused_) {
            self->state.max_lines_send = lines_received + self->state.max_outgoing_lines;
            self->state.paused = paused_;
            if (self->state.paused) {
                self->delayed_send(job_generator, std::chrono::milliseconds(1), checkpoint::value, self);
            } else {
                // continue reading..
                self->delayed_send(self, std::chrono::milliseconds(1), read_stdin::value);
            }
        },
        [=](debug) {
            aout(self) << "stdin_reader mailbox = " << self->mailbox().count()
                       << /*" " << self->mailbox().counter() << */endl;
        },
    };
}
