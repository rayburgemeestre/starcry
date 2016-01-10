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

#include "rendering_engine.hpp"

struct worker_data
{
    size_t worker_num = 0;
    ALLEGRO_BITMAP * bitmap = nullptr;
    rendering_engine engine;
    uint32_t width = 0;
    uint32_t height = 0;
};

behavior worker(caf::stateful_actor<worker_data> * self, const caf::actor &renderer, size_t worker_num) {
    self->state.worker_num = worker_num;
    return [=](get_job, struct data::job j) {
        // make sure our bitmap is of the correct size.
        if ((self->state.width == 0 && self->state.height == 0) || // not initialized
            (self->state.width != j.width || self->state.height != j.height) || // changed since previous
            self->state.bitmap == nullptr
        ){
            self->state.width = j.width;
            self->state.height = j.height;
            if (self->state.bitmap != nullptr) {
                al_destroy_bitmap(self->state.bitmap);
            }
            self->state.bitmap = al_create_bitmap(j.width, j.height);
        }
        // simulate work for rendering
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // render
        self->state.engine.render(self->state.bitmap, j.shapes);

        vector<ALLEGRO_COLOR> pixels = self->state.engine.serialize_bitmap(self->state.bitmap, j.width, j.height);

        self->send(renderer, ready::value, j, pixels);
    };
}

// move to class data
std::vector<data::job> jobs_done;
size_t job_sequence = 0;
std::unique_ptr<actor> pool;

auto benchmark_class = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter = static_cast<MeasureInterval &>(*benchmark_class.get());

//struct renderer_data
//{
//    rendering_engine engine;
//};

behavior renderer(event_based_actor* self, const caf::actor &job_storage, const caf::actor &streamer) {
//behavior renderer(caf::stateful_actor<renderer_data>* self, const caf::actor &job_storage, const caf::actor &streamer) {
    rendering_engine engine;
    engine.initialize();

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
        [=](ready, struct data::job j, vector<ALLEGRO_COLOR> pixels) {
            auto send_to_streamer = [&](struct data::job &job) {
                counter.measure();
                self->send(streamer, render_frame::value, job, pixels);
            };

            if (j.job_number == job_sequence) {
                send_to_streamer(j);
                job_sequence++;
                while (true) {
                    auto pos = find_if(jobs_done.begin(), jobs_done.end(), [&](auto &job) {
                        return job.job_number == job_sequence;
                    });
                    if (pos == jobs_done.end()) {
                        break;
                    }
                    send_to_streamer(*pos);
                    jobs_done.erase(pos);
                    job_sequence++;
                }
            } else {
                jobs_done.push_back(j);
            }

            self->send(self, render_frame::value);
        },
        [=](show_stats) {
            aout(self) << "renderer at job: " << job_sequence << ", with jobs/sec: " << (1000.0 / counter.mean())
                       << " +/- " << counter.stderr() << endl;
        }
    };
}

