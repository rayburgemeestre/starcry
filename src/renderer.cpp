#include "renderer.h"
#include "data/job.hpp"

extern void print_on_exit(const actor& hdl, const std::string& name);


size_t rendered_frame = 0;

behavior worker(event_based_actor* self, const caf::actor &job_storage, const caf::actor &renderer, size_t worker_num) {
	aout(self) << " generated worker instance \n";
	return [=](get_job_atom, struct data::job j) {
		aout(self) << "worker " << worker_num << " is rendering frame " << j.frame << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // simulating work
		aout(self) << ".. done" << endl;
		self->send(job_storage, remove_job_atom::value, j.frame);
		self->send(renderer, job_ready_atom::value, j);
	};
}

std::vector<int> frames_done;
behavior renderer(event_based_actor* self, const caf::actor &job_storage) {
	auto worker_factory = [&]() -> actor {
		static size_t worker_num = 0;
		return spawn(worker, job_storage, self, worker_num++);
	};
	auto pool = actor_pool::make(8, worker_factory, actor_pool::round_robin());
	self->link_to(pool);
	return {
		[=](start_rendering) {
			self->sync_send(job_storage, get_job_atom::value, rendered_frame).then(
				[=](job_not_available_atom) {
					aout(self) << "frame to render not available" << endl;
				},
				[=](get_job_atom, struct data::job j) {
					aout(self) << "delegating job to worker.." << endl;
					self->send(pool, get_job_atom::value, j);
					rendered_frame++;
				}
			);
		},
        [=](job_ready_atom, struct data::job j) {
            aout(self) << "~~job done = " << j.frame << endl;
            frames_done.push_back(j.frame);
            if (frames_done.size() == 25) {
                self->quit(exit_reason::user_shutdown);
            }
        }
	};
}

