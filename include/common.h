#pragma once

#include "caf/all.hpp"

using std::endl;
using namespace caf;

auto nothing = [](){};

inline void print_on_exit(const actor& hdl, const std::string& name) {
    hdl->attach_functor([=](abstract_actor *ptr, uint32_t reason) {
        aout(ptr) << name << " exited with reason " << reason << endl;
    });
}

#include <experimental/optional>

namespace std {
    using std::experimental::optional;
    using std::experimental::make_optional;
}
