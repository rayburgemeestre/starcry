#include "common.h"
#include "job_storage.h"
#include "data/job.hpp"

std::vector<data::job> jobs;

behavior job_storage(event_based_actor* self) {
    return {
        [=](get_job_atom, size_t frame) -> message {
            auto it = std::find_if (jobs.begin(), jobs.end(), [&frame](auto &job) {
                return job.frame == frame;
            });
            if (it == jobs.end()) {
                return make_message(job_not_available_atom::value);
            }
            return make_message(get_job_atom::value, *it);
        },
        [=](add_job_atom, size_t frame, bool rendered) {
            data::job new_job;
            new_job.frame = frame;
            new_job.rendered = rendered;
            jobs.push_back(new_job);
        },
        [=](remove_job_atom, size_t jobdone) {
            jobs.erase( std::remove_if(jobs.begin(), jobs.end(), [&jobdone](struct data::job j) {
                return j.frame == jobdone;
            }), jobs.end() );
        },
        [=](num_jobs_atom) -> message {
            return make_message(num_jobs_atom::value, jobs.size());
        }
    };
}
