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
std::vector<data::job> jobs_done;
size_t job_sequence = 0;
std::optional<size_t> last_frame;
std::unique_ptr<actor> pool;

auto benchmark_class = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter = static_cast<MeasureInterval &>(*benchmark_class.get());

behavior renderer(event_based_actor* self, const caf::actor &job_storage, const caf::actor &streamer) {
    counter.setDescription("fps");
    counter.startHistogramAtZero(true);
    return {
        [=](start, size_t num_workers) {
            auto worker_factory = [&]() -> actor {
                static size_t worker_num = 0;
                return spawn(worker, self, worker_num++);
            };
            pool = std::move(std::make_unique<actor>(actor_pool::make(num_workers, worker_factory, actor_pool::round_robin())));

            self->link_to(*pool);
            for (size_t i=0; i<num_workers; i++) self->send(self, render_frame::value);
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
            self->send(job_storage, del_job::value, j.job_number);

            if (j.last_frame) {
                last_frame = std::make_optional(j.job_number);
            }

            auto ready = [&](auto frame_number, auto chunk, auto num_chunks, bool last_frame) {
                counter.measure();
                self->send(streamer, render_frame::value, frame_number, chunk, num_chunks, last_frame);
            };

            if (j.job_number == job_sequence) {
                ready(j.frame_number, j.chunk, j.num_chunks, j.last_frame);
                job_sequence++;
                while (true) {
                    auto pos = find_if(jobs_done.begin(), jobs_done.end(), [&](auto &job) {
                        return job.job_number == job_sequence;
                    });
                    if (pos == jobs_done.end()) {
                        break;
                    }
                    ready(pos->frame_number, pos->chunk, pos->num_chunks, pos->last_frame);
                    jobs_done.erase(pos);
                    job_sequence++;
                }
            } else {
                jobs_done.push_back(j);
            }

            // todo, make a LAST_JOB as well... would be nicer!
//            if (last_frame && (job_sequence - 1) == *last_frame) {
//                self->quit(exit_reason::user_shutdown);
//            }

            self->send(self, render_frame::value);
        },
        [=](show_stats) {
            aout(self) << "renderer at job: " << job_sequence << ", with jobs/sec: " << (1000.0 / counter.mean())
                       << " +/- " << counter.stderr() << endl;
        }
    };
}

