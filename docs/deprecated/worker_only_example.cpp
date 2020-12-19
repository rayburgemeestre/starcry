// used older caf framework, needs to be updated
#include "common.h"
#include "actors/renderer.h"
#include "caf/io/middleman.hpp"
#include "data/pixels.hpp"
#include "data/job.hpp"
int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << "Usage " << argv[0] << " <worker_port>" << endl << endl;
    return 1;
  }
  actor_system_config cfg;

  cfg.add_message_type<data::job>("data::job");
  cfg.add_message_type<data::pixel_data>("data::pixel_data");
  cfg.add_message_type<data::pixel_data2>("data::pixel_data2");
  cfg.add_message_type<std::vector<uint32_t>>("vector<uint32_t>");
  cfg.load<io::middleman>();

  actor_system system(cfg);
  // TODO: remote renderer and/or streamer is not yet supported here
  auto w = system.spawn(remote_worker, atoi(argv[1]), "", 0, "", 0);
  return 0;
}
