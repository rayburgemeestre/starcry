#include "streamer.h"

#include "benchmark.h"

// public
using render_frame     = atom_constant<atom("render_fra")>;
using show_stats       = atom_constant<atom("show_stats")>;

// external
using del_job          = atom_constant<atom("del_job   ")>;

// internal
using process_queue    = atom_constant<atom("process_qu")>;
using ready            = atom_constant<atom("ready     ")>;

auto benchmark_class2 = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter2 = static_cast<MeasureInterval &>(*benchmark_class2.get());

size_t current_frame2 = 0; // initialize with a start ?

using namespace std;
vector<tuple<size_t,size_t,size_t,bool>> fake_buffer;
std::optional<size_t> last_frame_streamed;

#include "rendering_engine.hpp"

bool process_buffer(event_based_actor* self, size_t frame_number, size_t num_chunks) {
    if (count_if(fake_buffer.begin(),
                 fake_buffer.end(),
                 [&](auto &tpl) { return std::get<0>(tpl) == frame_number; }) == static_cast<int>(num_chunks)
    ){
        counter2.measure();
        fake_buffer.erase(std::remove_if(fake_buffer.begin(),
                                         fake_buffer.end(),
                                         [&](auto &tpl) { return std::get<0>(tpl) == frame_number; }),
                          fake_buffer.end());
        if (last_frame_streamed && *last_frame_streamed == frame_number) {
            aout(self) << "streamer completed frames: " << current_frame2 << ", with FPS: " << (1000.0 / counter2.mean())
                       << " +/- " << counter2.stderr() << endl;
            self->quit(exit_reason::user_shutdown);
        }
        return true;
    };
    return false;
}

#include "allegro5/allegro.h"
#include <allegro5/allegro_image.h>
#include "data/job.hpp"

behavior streamer(event_based_actor* self, const caf::actor &job_storage) {
    counter2.setDescription("fps");
    counter2.startHistogramAtZero(true);
    return {
        [=](render_frame, struct data::job &job, vector<ALLEGRO_COLOR> &pixels) {
            if (job.last_frame)
                last_frame_streamed = std::make_optional(job.frame_number);

            rendering_engine e;
            ALLEGRO_BITMAP *bitmap = e.unserialize_bitmap(pixels, job.width, job.height);

            al_destroy_bitmap(bitmap);

            fake_buffer.push_back(make_tuple(job.frame_number, job.chunk, job.num_chunks, job.last_frame)); // needs to become an object later
            while (process_buffer(self, current_frame2, job.num_chunks))
                current_frame2++;

            // return make_message(ready::value);
            self->send(job_storage, del_job::value, job.job_number);
        },
        [=](show_stats) {
            aout(self) << "streamer at frame: " << current_frame2 << ", with FPS: " << (1000.0 / counter2.mean())
                       << " +/- " << counter2.stderr() << endl;
        }
    };
}
