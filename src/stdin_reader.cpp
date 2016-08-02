/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "actors/stdin_reader.h"

using start            = atom_constant<atom("start     ")>;
using input_line       = atom_constant<atom("input_line")>;
using read_stdin       = atom_constant<atom("read_stdin")>;
using num_jobs         = atom_constant<atom("num_jobs  ")>;
using no_more_input    = atom_constant<atom("no_more_in")>;

int max_num_lines_batch = 5000;
int desired_max_mailbox_generator = 1000;
//extern size_t desired_num_jobs_queued;

using namespace std;

/**
 * This standard input reader is pretty stupid, it doesn't properly rate-limit yet..
 *  it takes the job generator a while to start filling it's queue, by the time it's ready
 *  this actor already "piped" all the stdin into the job_generator's mailbox.
 */
behavior stdin_reader(event_based_actor *self, const caf::actor &job_generator, const caf::actor &job_storage) {
    self->link_to(job_generator);
    self->link_to(job_storage);
    return {
        [=](start) {
            self->send(self, read_stdin::value);
        },
        [=](read_stdin) {
            self->request(job_generator, infinite, num_jobs::value).then( // TODO: rename mailbox_size::value ?
                [=](num_jobs, unsigned long numjobs) {
                    if (numjobs >= desired_max_mailbox_generator) {
                        self->send(self, read_stdin::value);
                        return;
                    }

                    string line;
                    // TODO: make this asynchronous.. so stdin_reader can be pulled from a different actor, and also exited in case enough input is gathered..
                    for (int i=0; i<max_num_lines_batch; i++) {
                        if (!getline(cin, line)) {
                            aout(self) << "stdin_reader: EOF" << endl;
                            self->send(job_generator, no_more_input::value);
                            return;
                        }
                        self->send(job_generator, input_line::value, line);
                    }
                    self->send(self, read_stdin::value);
                }
            );
        }
    };
}
