#include "renderer.h"
#include "data/job.hpp"

#include "benchmark.h"

// public
using start                = atom_constant<atom("start     ")>;
using show_stats           = atom_constant<atom("show_stats")>;

// external
using get_job              = atom_constant<atom("get_job   ")>;
using del_job              = atom_constant<atom("del_job   ")>;
using no_jobs_available    = atom_constant<atom("no_jobs_av")>;

// internal
using ready                = atom_constant<atom("ready     ")>;
using render_frame         = atom_constant<atom("render_fra")>;

size_t rendered_frame = 0;

behavior worker(event_based_actor* self, const caf::actor &renderer, size_t worker_num) {
    return [=](get_job, struct data::job j) {
        // simulate work for rendering
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        self->send(renderer, ready::value, j);
    };
}

// move to class data
std::vector<data::job> frames_done;
size_t frame_sequence = 0;
std::optional<size_t> last_frame;
std::unique_ptr<actor> pool;

auto benchmark_class = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter = static_cast<MeasureInterval &>(*benchmark_class.get());

behavior renderer(event_based_actor* self, const caf::actor &job_storage, const caf::actor &streamer) {
    counter.setDescription("fps");
    counter.startHistogramAtZero(true);
    return {
        [=](start, size_t cores) {
            auto worker_factory = [&]() -> actor {
                static size_t worker_num = 0;
                return spawn(worker, self, worker_num++);
            };
            pool = std::move(std::make_unique<actor>(actor_pool::make(cores, worker_factory, actor_pool::round_robin())));

            self->link_to(*pool);
            for (size_t i=0; i<cores; i++) self->send(self, render_frame::value);
        },
        [=](render_frame) {
            self->sync_send(job_storage, get_job::value, rendered_frame).then(
                [=](no_jobs_available) {
                    self->send(self, render_frame::value);
                },
                [=](get_job, struct data::job j) {
                    self->send(*pool, get_job::value, j);
                    rendered_frame++;
                }
            );
        },
        [=](ready, struct data::job j) {
            self->send(job_storage, del_job::value, j.frame);

            //aout(self) << "renderer: rendered frame: " << j.frame << endl;
            if (j.last_frame) {
                last_frame = std::make_optional(j.frame);
            }

            auto ready = [&](auto &frameNumber) {
                counter.measure();
                self->send(streamer, render_frame::value, frameNumber);
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
        [=](show_stats) {
            aout(self) << "renderer at frame: " << frame_sequence << ", with FPS: " << (1000.0 / counter.mean())
                       << " +/- " << counter.stderr() << endl;
        }
    };
}

