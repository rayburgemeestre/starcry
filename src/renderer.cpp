#include "renderer.h"

size_t rendered_frame = 0;
behavior renderer(event_based_actor* self, const caf::actor &job_storage) {
	return {
		[=](start_rendering) {
			self->sync_send(job_storage, get_job_atom::value, rendered_frame).then(
				[=](job_not_available_atom) {
					aout(self) << "frame to render not available" << endl;
				},
				[=](get_job_atom, unsigned long frame) {
					aout(self) << "rendering frame " << frame;
					std::this_thread::sleep_for(std::chrono::milliseconds(80));
					aout(self) << "done" << endl;
					self->send(job_storage, remove_job_atom::value, frame);
					rendered_frame++;
					if (rendered_frame == 25) {
						self->quit(exit_reason::user_shutdown);
					}
				}
			);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	};
}
