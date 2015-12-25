#include <iostream>

#include "common.h"
#include "job_storage.h"
#include "job_generator.h"
#include "renderer.h"

#include "util/actor_info.hpp"

using start_rendering = atom_constant<atom("startrende")>;

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
    s->send(renderer_, start_rendering::value);

    while (renderer_info.running()) {
        std::cout << " rendering still ongoing.. \n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    s->await_all_other_actors_done();
}
