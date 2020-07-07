/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry_interactive.h"

#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>

// configured in interactive_starcry::stop()
void my_handler(int s) {
  printf("Caught signal %d\n", s);
  exit(1);
}

interactive_starcry::interactive_starcry()
    : system(true),
      ws(this),
      cmds(system.create_queue(10)),
      jobs(system.create_queue(10)),
      frames(system.create_queue(10)) {
  system.spawn_transformer<instruction>(
      [&](auto job) -> auto {
        // std::cout << "converting instruction to job" << std::endl;
        return job;
      },
      cmds,
      jobs);

  system.spawn_transformer<instruction>(
      [&](auto job) -> auto {
        // std::cout << "converting job to frame" << std::endl;
        return job;
      },
      jobs,
      frames);

  system.spawn_consumer<message_type>(
      [](auto) {
        // std::cout << "todo: pass to client this frame?" << std::endl;
      },
      frames);
  system.start();
}

void interactive_starcry::add_command() {
  cmds->push(std::make_shared<instruction>());
}

/**
 * Called right after the CROW webserver stops. CROW overrides the signal handler,
 * so we have to set a new one here to allow the user to interrupt our graceful shutdown.
 */
void interactive_starcry::stop() {
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
  system.visualization_enabled = false;
  std::cout << std::endl
            << "Gracefully shutting down pipeline.." << std::endl
            << "Press [control]+[c] again to force shutdown." << std::endl;
  cmds->check_terminate();
}
