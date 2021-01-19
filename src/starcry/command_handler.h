/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

class starcry;
class instruction;
class render_msg;
class job_message;
class image;

class command_handler {
protected:
  starcry &sc;

public:
  command_handler(starcry &sc);
  virtual ~command_handler() = default;

  // same for get_image, get_shapes, get_objects, get_bitmap, different for get_video
  virtual void to_job(std::shared_ptr<instruction> &cmd_def);

  virtual std::shared_ptr<render_msg> to_render_msg(std::shared_ptr<job_message> &cmd_def, image &bmp);

  virtual void handle_frame(std::shared_ptr<render_msg> &job_msg);
};
