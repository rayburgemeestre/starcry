#include <string>

bool connect_remote_worker(actor_system &system, const std::string &actor_name, const std::string &host, const int &port, std::optional<actor> *actor_ptr);

bool publish_remote_actor(const std::string &actor_name, event_based_actor *self, int port);

