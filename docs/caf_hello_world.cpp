#include "caf/all.hpp"

using namespace caf;

CAF_BEGIN_TYPE_ID_BLOCK(test, first_custom_type_id)
CAF_ADD_ATOM(test, test)
CAF_END_TYPE_ID_BLOCK(test)

behavior bar_actor(event_based_actor* self) {
  return {
    [=](test) {
      std::cout << "test1" << std::endl;
    },
    [=](uint32_t value) {
      std::cout << "test2: " << value << std::endl;
    }
  };
}

behavior foo_actor(event_based_actor* self) {
  auto bar = self->spawn(bar_actor);
  bar->add_link(self);
  self->link_to(bar);
  self->send(bar, test_v); // segfaults

  return {
    [=](uint32_t width) {
      self->send(bar, test_v); // hangs (when by itself)
      self->send(bar, width);  // segfaults
    },
  };
}

void caf_main(actor_system& system) {
  scoped_actor s{system};
  auto foo = system.spawn(foo_actor);
  s->send(foo, uint32_t(123));
  s->await_all_other_actors_done();
}

// creates a main function for us that calls our caf_main
CAF_MAIN(caf::id_block::test)
