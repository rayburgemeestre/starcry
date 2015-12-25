#include <iostream>

#include "common.h"
#include "job_storage.h"
#include "job_generator.h"
#include "renderer.h"

#include "util/actor_info.hpp"

using start_rendering = atom_constant<atom("startrende")>;
using stats_atom = atom_constant<atom("stats")>;

int main() {
    scoped_actor s;
    auto jobstorage = spawn(job_storage);
    auto generator  = spawn(job_generator, jobstorage);
    auto renderer_  = spawn(renderer, jobstorage);

    // cascade exit from renderer -> job renderer -> job storage
    generator->link_to(renderer_);
    jobstorage->link_to(generator);

    actor_info renderer_info{renderer_};
    s->send(generator, prepare_frame::value);

    size_t num_cores = 100;
    s->send(renderer_, start_rendering::value, num_cores);

    while (renderer_info.running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        s->send(renderer_, stats_atom::value);
    }
    s->await_all_other_actors_done();
}
