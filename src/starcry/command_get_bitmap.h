/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "starcry/command_handler.h"

class command_get_bitmap : public command_handler {
public:
  command_get_bitmap(starcry& sc);

  void to_job(std::shared_ptr<instruction>& cmd_def);

  std::shared_ptr<render_msg> to_render_msg(std::shared_ptr<job_message>& cmd_def, image& bmp);

  void handle_frame(std::shared_ptr<render_msg>& job_msg);
};
