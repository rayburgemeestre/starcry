/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "common.h"
#include "actors/job_storage.h"
#include "data/job.hpp"
#include "caf/none.hpp"

// public
using get_job              = atom_constant<atom("get_job   ")>;
using add_job              = atom_constant<atom("add_job   ")>;
using del_job              = atom_constant<atom("del_job   ")>;
using num_jobs             = atom_constant<atom("num_jobs  ")>;
using no_jobs_available    = atom_constant<atom("no_jobs_av")>;

std::vector<data::job> jobs;

behavior job_storage(event_based_actor* self) {
    return {
        [=](get_job, size_t job) -> optional<data::job> {
            auto it = std::find_if (jobs.begin(), jobs.end(), [&job](auto &j) {
                return j.job_number == job;
            });
            if (it == jobs.end()) {
                return none;
            }
            return *it;
        },
        [=](add_job, data::job new_job) {
            jobs.push_back(new_job);
        },
        [=](del_job, size_t jobdone) {
            jobs.erase( std::remove_if(jobs.begin(), jobs.end(), [&jobdone](struct data::job j) {
                return j.job_number == jobdone;
            }), jobs.end() );
        },
        [=](num_jobs) -> message {
            return make_message(num_jobs::value, jobs.size());
        }
    };
}
