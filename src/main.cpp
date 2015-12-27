#include <iostream>

#include "common.h"
#include "job_storage.h"
#include "job_generator.h"
#include "renderer.h"
#include "streamer.h"

#include "util/actor_info.hpp"

using start         = atom_constant<atom("start     ")>;
using show_stats    = atom_constant<atom("show_stats")>;

int main() {
    scoped_actor s;
    auto jobstorage = spawn(job_storage);
    auto generator  = spawn(job_generator, jobstorage);
    auto streamer_  = spawn(streamer);
    auto renderer_  = spawn(renderer, jobstorage, streamer_);

    // cascade exit from renderer -> job generator -> job storage
    generator->link_to(renderer_);
    jobstorage->link_to(generator);
    streamer_->link_to(renderer_);

    actor_info renderer_info{renderer_};
    size_t num_chunks  = 8; // number of chunks to split image into
    size_t num_workers = 8; // number of workers for rendering
    s->send(generator, start::value, num_chunks);
    s->send(renderer_, start::value, num_workers);

    while (renderer_info.running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        s->send(renderer_, show_stats::value);
        s->send(streamer_, show_stats::value);
    }
    s->await_all_other_actors_done();
}
