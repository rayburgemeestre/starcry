/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry/command_handler.h"

#include "generator_v2.h"
#include "messages.hpp"
#include "starcry.h"

command_handler::command_handler(starcry &sc) : sc(sc) {}

void command_handler::to_job(std::shared_ptr<instruction> &cmd_def) {
  size_t idx = 0;
  while (sc.gen->generate_frame()) {
    if (++idx >= cmd_def->frame) {
      break;
    }
  }
  auto the_job = sc.gen->get_job();
  the_job->job_number = std::numeric_limits<uint32_t>::max();
  sc.jobs->push(std::make_shared<job_message>(cmd_def->client, cmd_def->type, the_job));
}

std::shared_ptr<render_msg> command_handler::to_render_msg(std::shared_ptr<job_message> &cmd_def, image &bmp) {
  return nullptr;
}

void command_handler::handle_frame(std::shared_ptr<render_msg> &job_msg) {}
