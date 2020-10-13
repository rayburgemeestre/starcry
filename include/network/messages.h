/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

enum starcry_msgs {
  register_me = 10,
  register_me_response,
  pull_job = 20,
  pull_job_response,
  send_frame = 30,
  send_frame_response,
};