#include <iostream>
#include <atomic>

#include "common.h"
#include "job_storage.h"
#include "job_generator.h"
#include "renderer.h"

void print_on_exit(const actor& hdl, const std::string& name) {
    hdl->attach_functor([=](abstract_actor *ptr, uint32_t reason) {
        aout(ptr) << name << " exited with reason " << reason << endl;
    });
}

int main() {
    scoped_actor s;
    auto jobstorage = spawn(job_storage);
    auto generator  = spawn(job_generator, jobstorage);
    auto renderer_  = spawn(renderer, jobstorage);
    print_on_exit(jobstorage, "jobstorage");
    print_on_exit(generator, "generator");
    print_on_exit(renderer_, "renderer");

    // cascade exit from renderer -> job renderer -> job storage
    generator->link_to(renderer_);
    jobstorage->link_to(generator);

    // run flag is true while renderer is still alive
    std::atomic<bool> run{true};
    renderer_->attach_functor([=, &run](abstract_actor* ptr, uint32_t reason) {
        aout(ptr) << "renderer" << " exited with reason " << reason << endl;
        run.store(false);
    });

    while (run) {
        s->send(renderer_, start_rendering::value); // tries to get frames from job_storage for rendering
        s->send(generator, create_jobs_atom::value); // generates frames and stores them into job_storage
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    aout(s) << "running.. END " << endl;
    s->await_all_other_actors_done();
}
