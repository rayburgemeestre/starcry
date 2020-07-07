/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>

#include "piper.h"
#include "webserver.h"

class instruction : public message_type {};

class interactive_starcry {
private:
  pipeline_system system;
  webserver ws;
  std::shared_ptr<queue> cmds;
  std::shared_ptr<queue> jobs;
  std::shared_ptr<queue> frames;

public:
  interactive_starcry();

  void add_command();
  void stop();
};
