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
using debug                = atom_constant<atom("debug     ")>;

std::set<data::job> jobs;

behavior job_storage(event_based_actor* self) {
    return {
        [=](get_job, const caf::actor &sender) {
            std::cout << "job_storage got request for a job " << endl;
            if (jobs.empty()) {
                self->send(sender, get_job::value, false);
                return;
            }
            auto &j  =(*jobs.cbegin());
            std::cout << "sending job.." << j.job_number << endl;
            self->send(sender, get_job::value, j);
            std::cout << "removing job already..." << endl;
            jobs.erase(j);
        },
        [=](get_job, size_t job, const caf::actor &sender) /*-> optional<data::job>*/ {
            std::cout << "job_storage got request for job: " << job << endl;
            auto it = std::find_if (jobs.begin(), jobs.end(), [&job](auto &j) {
                return j.job_number == job;
            });
            if (it == jobs.end()) {
                //return none;
                self->send(sender, get_job::value, false);
                return;
            }
            //return *it;
            std::cout << "sending job.." << (*it).job_number << endl;
            self->send(sender, get_job::value, *it);
        },
        [=](add_job, data::job new_job) {
            jobs.insert(new_job);
        },
        [=](del_job, size_t jobdone) {
//            auto f = std::find_if(jobs.begin(), jobs.end(), [&jobdone](struct data::job j) {
//                return j.job_number == jobdone;
//            });
//            if (f != jobs.end())
//                jobs.erase(*f);
        },
        [=](num_jobs) -> message {
            return make_message(num_jobs::value, jobs.size());
        },
        [=](debug) {
            aout(self) << "job_storage mailbox = " << self->mailbox().count() << " " << self->mailbox().counter() << endl;
        }
    };
}
