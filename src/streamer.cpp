#include "streamer.h"

#include "benchmark.h"

// public
using render_frame     = atom_constant<atom("render_fra")>;
using show_stats       = atom_constant<atom("show_stats")>;

// internal
using process_queue    = atom_constant<atom("process_qu")>;

auto benchmark_class2 = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter2 = static_cast<MeasureInterval &>(*benchmark_class2.get());

size_t current_frame2 = 0; // initialize with a start ?

using namespace std;
vector<tuple<size_t,size_t,size_t,bool>> fake_buffer;
std::optional<size_t> last_frame_streamed;

bool process_buffer(event_based_actor* self, size_t frame_number, size_t num_chunks) {
    if (count_if(fake_buffer.begin(),
                 fake_buffer.end(),
                 [&](auto &tpl) { return std::get<0>(tpl) == frame_number; }) == num_chunks
    ){
        counter2.measure();
        fake_buffer.erase(std::remove_if(fake_buffer.begin(),
                                         fake_buffer.end(),
                                         [&](auto &tpl) { return std::get<0>(tpl) == frame_number; }),
                          fake_buffer.end());
        if (last_frame_streamed && *last_frame_streamed == frame_number) {
            self->quit(exit_reason::user_shutdown);
        }
        return true;
    };
    return false;
}

behavior streamer(event_based_actor* self) {
    counter2.setDescription("fps");
    counter2.startHistogramAtZero(true);
    return {
        [=](render_frame, size_t frame_num, size_t chunk, size_t num_chunks, bool last_frame) {
            if (last_frame)
                last_frame_streamed = std::make_optional(frame_num);
            fake_buffer.push_back(make_tuple(frame_num, chunk, num_chunks, last_frame)); // needs to become an object later
            while (process_buffer(self, current_frame2, num_chunks))
                current_frame2++;
        },
        [=](show_stats) {
            aout(self) << "streamer at frame: " << current_frame2 << ", with FPS: " << (1000.0 / counter2.mean())
                       << " +/- " << counter2.stderr() << endl;
        }
    };
}
