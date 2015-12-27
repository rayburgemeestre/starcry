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

#include "allegro5/allegro.h"
#include <allegro5/allegro_image.h>

#include <mutex>

using namespace std;
class rendering_engine
{
public:
    void initialize() {
        if (!al_init()) {
            fprintf(stderr, "Failed to initialize allegro!\n");
            return;
        }
        if (!al_init_image_addon()) {
            fprintf(stderr, "Failed to initialize allegro!\n");
            return;
        }
    }
    template <typename image, typename shapes_t>
    void render(image bmp, shapes_t & shapes) {
        std::unique_lock<std::mutex> lock(m);
        al_set_target_bitmap(bmp);
        for (auto &shape : shapes) {
            al_clear_to_color(al_map_rgba(shape.radius, 255, 0, 0));
        }
    }
    void write_image() {
        std::unique_lock<std::mutex> lock(m);
        //al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
        /*bool ret = al_save_bitmap("/tmp/test.bmp", bmp);
        if (ret) cout << " ret is ok" <<endl;
        else cout << "ret is nok" << endl;
         */
        //al_unlock_bitmap(bmp);

        //
    }
    ALLEGRO_DISPLAY *display = NULL;
    std::mutex m;
};

struct worker_data // still necessary?
{
    size_t worker_num = 0;
    ALLEGRO_BITMAP * bitmap = NULL;
    rendering_engine engine;
};

behavior worker(caf::stateful_actor<worker_data> * self, const caf::actor &renderer, size_t worker_num) {
    self->state.worker_num = worker_num;
    self->state.bitmap = al_create_bitmap(800, 600);
    return [=](get_job, struct data::job j) {
        // simulate work for rendering
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //aout(self) << "worker sees job with radius: " << j.shapes[0].radius << " - " << self->state.worker_num << endl;
        self->state.engine.render(self->state.bitmap, j.shapes);
        vector<ALLEGRO_COLOR> pixels;
        pixels.reserve(800 * 600);
        al_lock_bitmap(self->state.bitmap, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
        for (int y=0; y<600; y++) {
            for (int x=0; x<800; x++) {
                pixels.emplace_back(al_get_pixel(self->state.bitmap, x, y)); // interesting this one has a bitmap arg??
            }
        }
        al_unlock_bitmap(self->state.bitmap);
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
            self->send(job_storage, del_job::value, j.job_number);

            auto ready = [&](auto frame_number, auto chunk, auto num_chunks, bool last_frame) {
                counter.measure();
                self->send(streamer, render_frame::value, frame_number, chunk, num_chunks, last_frame, pixels);
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

            self->send(self, render_frame::value);
        },
        [=](show_stats) {
            aout(self) << "renderer at job: " << job_sequence << ", with jobs/sec: " << (1000.0 / counter.mean())
                       << " +/- " << counter.stderr() << endl;
        }
    };
}

