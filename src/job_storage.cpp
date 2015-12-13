#include "common.h"
#include "job_storage.h"

struct job { size_t frame; bool rendered; };
std::vector<job> jobs;

behavior job_storage(event_based_actor* self) {
	return {
		[=](get_job_atom, size_t frame) -> message {
			auto it = std::find_if (jobs.begin(), jobs.end(), [&frame](auto &job) {
				return job.frame == frame;
			});
			if (it == jobs.end()) {
				return make_message(job_not_available_atom::value);
			}
			return make_message(get_job_atom::value, (unsigned long)it->frame);
		},
		[=](add_job_atom, size_t frame, bool rendered) -> message {
			if (frame > 25) {
				return make_message(no_more_jobs_atom::value);
			}
			job new_job;
			new_job.frame = frame;
			new_job.rendered = rendered;
			jobs.push_back(new_job);
			aout(self) << "prepared job for frame: " << new_job.frame << endl;
			return make_message(num_jobs_atom::value, jobs.size());
		},
		[=](remove_job_atom, size_t jobdone) -> message {
			aout(self) << "removing job for frame: " << jobdone<< endl;
			jobs.erase( std::remove_if(jobs.begin(), jobs.end(), [&jobdone](struct job j) {
				return j.frame == jobdone;
			}), jobs.end() );
			return make_message(num_jobs_atom::value, jobs.size());
		},
		[=](num_jobs_atom) -> message {
			return make_message(num_jobs_atom::value, jobs.size());
		}
	};
}
