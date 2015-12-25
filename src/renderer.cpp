#include "renderer.h"
#include "data/job.hpp"

using start_rendering = atom_constant<atom("startrende")>;
using render_frame = atom_constant<atom("renderfram")>;
using get_job_atom = atom_constant<atom("getjob")>;
using remove_job_atom = atom_constant<atom("removejob")>;
using job_not_available_atom = atom_constant<atom("job_na")>;
using job_ready_atom = atom_constant<atom("job_rdy")>;

size_t rendered_frame = 0;

behavior worker(event_based_actor* self, const caf::actor &job_storage, const caf::actor &renderer, size_t worker_num) {
    return [=](get_job_atom, struct data::job j) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // simulating work
        self->send(job_storage, remove_job_atom::value, j.frame);
        self->send(renderer, job_ready_atom::value, j);
    };
}

// move to class data
std::vector<data::job> frames_done;
size_t cores = 8;
size_t frame_sequence = 0;
behavior renderer(event_based_actor* self, const caf::actor &job_storage) {
    auto worker_factory = [&]() -> actor {
        static size_t worker_num = 0;
        return spawn(worker, job_storage, self, worker_num++);
    };
    auto pool = actor_pool::make(cores, worker_factory, actor_pool::round_robin());
    self->link_to(pool);
    return {
        [=](start_rendering) {
            for (size_t i=0; i<cores; i++) self->send(self, render_frame::value);
        },
        [=](render_frame) {
            self->sync_send(job_storage, get_job_atom::value, rendered_frame).then(
                [=](job_not_available_atom) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    self->send(self, render_frame::value);
                },
                [=](get_job_atom, struct data::job j) {
                    self->send(pool, get_job_atom::value, j);
                    rendered_frame++;
                }
            );
        },
        [=](job_ready_atom, struct data::job j) {
            aout(self) << "renderer: rendered frame: " << j.frame << endl;

            if (j.frame == frame_sequence) {
                aout(self) << "renderer: synchronized sending frame: " << j.frame << endl;
                frame_sequence++;
                while (true) {
                    auto pos = find_if(frames_done.begin(), frames_done.end(), [&](auto &job) {
                        return job.frame == frame_sequence;
                    });
                    if (pos == frames_done.end()) {
                        break;
                    }
                    frames_done.erase(pos);
                    aout(self) << "renderer: synchronized sending frame~~: " << pos->frame << endl;
                    frame_sequence++;
                }
            } else {
                frames_done.push_back(j);
            }

            if (frame_sequence >= 25) {
                self->quit(exit_reason::user_shutdown);
            }

            aout(self) << " number of jobs in buffer: " << frames_done.size() << endl;
            self->send(self, render_frame::value);
        }
    };
}

