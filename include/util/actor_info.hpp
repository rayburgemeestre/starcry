#pragma once
#include <atomic>

class actor_info
{
public:
    std::atomic<bool> flag_;
    actor_info(const caf::actor &actor) : flag_(true) {
        actor->attach_functor([=](abstract_actor* ptr, uint32_t reason) {
            aout(ptr) << "renderer" << " exited with reason " << reason << endl;
            flag_.store(false);
        });
    }
    bool running () { return flag_; }
};
