#include "caf/io/all.hpp"
#include "common.h"

using namespace std;

bool connect_remote_worker(actor_system &system,
                           const string &actor_name,
                           const string &host,
                           const int &port,
                           std::optional<actor> *actor_ptr) {
  if (port) {
    cout << "connecting to " << actor_name << " at " << host << " : " << port << endl;
    auto p = system.middleman().remote_actor(host, port);
    if (!p) {
      cout << "connecting to " << actor_name << " failed: " << system.render(p.error()) << endl;
      return false;
    }
    *actor_ptr = *p;
    return true;
  }
  return true;
}

bool publish_remote_actor(const string &actor_name, event_based_actor *self, int port) {
  if (!port) return false;

  aout(self) << "publishing " << actor_name << " on port: " << port << endl;
  auto p = self->system().middleman().publish(static_cast<actor>(self), port, nullptr, true);
  if (!p) {
    aout(self) << "publishing " << actor_name << " FAILED..: " << self->system().render(p.error()) << endl;
    return false;
  } else if (*p != port) {
    aout(self) << "publishing " << actor_name << " FAILED.." << endl;
    return false;
  }
  return true;
}
