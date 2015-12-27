#include "streamer.h"

#include "benchmark.h"

// public
using render_frame    = atom_constant<atom("render_fra")>;
using show_stats      = atom_constant<atom("show_stats")>;

auto benchmark_class2 = std::make_unique<MeasureInterval>(TimerFactory::Type::BoostChronoTimerImpl);
MeasureInterval &counter2 = static_cast<MeasureInterval &>(*benchmark_class2.get());

size_t current_frame2 = 0;

behavior streamer(event_based_actor* self) {
    counter2.setDescription("fps");
    counter2.startHistogramAtZero(true);
    return {
        [=](render_frame, size_t frame_num) {
            current_frame2 = frame_num;
            counter2.measure();
            //aout(self) << "in streamer rendering frame" << frame_num << endl;
        },
        [=](show_stats) {
            aout(self) << "streamer at frame: " << current_frame2 << ", with FPS: " << (1000.0 / counter2.mean())
                       << " +/- " << counter2.stderr() << endl;
        }
    };
}
