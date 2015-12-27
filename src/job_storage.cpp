#include "common.h"
#include "job_storage.h"
#include "data/job.hpp"

// public
using get_job              = atom_constant<atom("get_job   ")>;
using add_job              = atom_constant<atom("add_job   ")>;
using del_job              = atom_constant<atom("del_job   ")>;
using num_jobs             = atom_constant<atom("num_jobs  ")>;
using no_jobs_available    = atom_constant<atom("no_jobs_av")>;

std::vector<data::job> jobs;

behavior job_storage(event_based_actor* self) {
    return {
        [=](get_job, size_t frame) -> message {
            auto it = std::find_if (jobs.begin(), jobs.end(), [&frame](auto &job) {
                return job.frame == frame;
            });
            if (it == jobs.end()) {
                return make_message(no_jobs_available::value);
            }
            return make_message(get_job::value, *it);
        },
        [=](add_job, size_t frame, bool rendered, bool last_frame) {
            data::job new_job;
            new_job.frame = frame;
            new_job.rendered = rendered;
            new_job.last_frame = last_frame;
            jobs.push_back(new_job);
        },
        [=](del_job, size_t jobdone) {
            jobs.erase( std::remove_if(jobs.begin(), jobs.end(), [&jobdone](struct data::job j) {
                return j.frame == jobdone;
            }), jobs.end() );
        },
        [=](num_jobs) -> message {
            return make_message(num_jobs::value, jobs.size());
        }
    };
}
