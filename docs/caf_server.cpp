#include "caf/all.hpp"
#include "caf/io/all.hpp"

using namespace caf;

CAF_BEGIN_TYPE_ID_BLOCK(test, first_custom_type_id)
CAF_ADD_ATOM(test, init)
CAF_ADD_ATOM(test, ping)
CAF_ADD_ATOM(test, pong)
CAF_END_TYPE_ID_BLOCK(test)

behavior pingable(event_based_actor* self) {
  self->system().middleman().publish(self, 12345, nullptr, true);
  return {
    [=](ping, caf::actor &sender) {
      std::cout << "PING" << std::endl;
      for (auto i = 0; i < 10; i++) {
        self->send(sender, pong_v);
      }
    }
  };
}

void caf_main(actor_system& system) {
  scoped_actor s{system};
  auto p = system.spawn(pingable);
  // s->send(p, ping_v);
  s->await_all_other_actors_done();
}

CAF_MAIN(caf::id_block::test, io::middleman)
