#include "renderer.h"
#include "data/job.hpp"

#include "benchmark.h"

using start_rendering         = atom_constant<atom("startrende")>;
using render_frame            = atom_constant<atom("renderfram")>;
using get_job_atom            = atom_constant<atom("getjob")>;
using remove_job_atom         = atom_constant<atom("removejob")>;
using job_not_available_atom  = atom_constant<atom("job_na")>;
using job_ready_atom          = atom_constant<atom("job_rdy")>;
using stats_atom              = atom_constant<atom("stats")>;

size_t rendered_frame = 0;

behavior worker(event_based_actor* self, const caf::actor &job_storage, const caf::actor &renderer, size_t worker_num) {
    return [=](get_job_atom, struct data::job j) {
        // simulate work for rendering
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        self->send(job_storage, remove_job_atom::value, j.frame);
        self->send(renderer, job_ready_atom::value, j);
    };
}

// move to class data
std::vector<data::job> frames_done;
size_t frame_sequence = 0;
std::optional<size_t> last_frame;
std::unique_ptr<actor> pool;

auto benchmark_class = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter = static_cast<MeasureInterval &>(*benchmark_class.get());

behavior renderer(event_based_actor* self, const caf::actor &job_storage) {
    counter.setDescription("fps");
    counter.startHistogramAtZero(true);
    return {
        [=](start_rendering, size_t cores) {
            auto worker_factory = [&]() -> actor {
                static size_t worker_num = 0;
                return spawn(worker, job_storage, self, worker_num++);
            };
            pool = std::move(std::make_unique<actor>(actor_pool::make(cores, worker_factory, actor_pool::round_robin())));

            self->link_to(*pool);
            for (size_t i=0; i<cores; i++) self->send(self, render_frame::value);
        },
        [=](render_frame) {
            self->sync_send(job_storage, get_job_atom::value, rendered_frame).then(
                [=](job_not_available_atom) {
                    // aout(self) << "[renderer] WARNING: storage has no jobs available" << endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(8));
                    self->send(self, render_frame::value);
                },
                [=](get_job_atom, struct data::job j) {
                    self->send(*pool, get_job_atom::value, j);
                    rendered_frame++;
                }
            );
        },
        [=](job_ready_atom, struct data::job j) {
            //aout(self) << "renderer: rendered frame: " << j.frame << endl;
            if (j.last_frame) {
                last_frame = std::make_optional(j.frame);
            }

            auto ready = [&](auto &frameNumber) {
                counter.measure();
                //aout(self) << "renderer: synchronized sending frame: " << frameNumber << endl;
            };

            if (j.frame == frame_sequence) {
                ready(j.frame);
                frame_sequence++;
                while (true) {
                    auto pos = find_if(frames_done.begin(), frames_done.end(), [&](auto &job) {
                        return job.frame == frame_sequence;
                    });
                    if (pos == frames_done.end()) {
                        break;
                    }
                    frames_done.erase(pos);
                    ready(pos->frame);
                    frame_sequence++;
                }
            } else {
                frames_done.push_back(j);
            }

            if (last_frame && (frame_sequence - 1) == *last_frame) {
                self->quit(exit_reason::user_shutdown);
            }

            self->send(self, render_frame::value);
        },
        [=](stats_atom) {
            aout(self) << "renderer at frame: " << frame_sequence << ", with FPS: " << (1000.0 / counter.mean())
                       << " +/- " << counter.stderr() << endl;
        }
    };
}

