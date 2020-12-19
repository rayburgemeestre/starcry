#include "caf/all.hpp"
#include "caf/io/all.hpp"

using namespace caf;

CAF_BEGIN_TYPE_ID_BLOCK(test, first_custom_type_id)
CAF_ADD_ATOM(test, init)
CAF_ADD_ATOM(test, ping)
CAF_ADD_ATOM(test, pong)
CAF_END_TYPE_ID_BLOCK(test)

behavior pinger(event_based_actor* self) {
  auto server = *self->system().middleman().remote_actor("127.0.0.1", 12345);
  return {
    [=](init) {
      self->send(server, ping_v, self);
    },
    [=](pong) -> caf::message {
      std::cout << "PONG" << std::endl;
      return caf::make_message(pong_v);
    }
  };
}

void caf_main(actor_system& system) {
  scoped_actor s{system};
  auto p = system.spawn(pinger);
  s->send(p, init_v);
  s->await_all_other_actors_done();
}

CAF_MAIN(caf::id_block::test, io::middleman)
