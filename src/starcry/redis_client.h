/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

namespace sw {
namespace redis {
class Redis;
}
}  // namespace sw
class bitmap_wrapper;
class starcry;
class rendering_engine;

class redis_client {
private:
  std::unique_ptr<sw::redis::Redis> redis;
  std::string my_id_;
  std::string host;
  std::string known_server_id_;
  starcry& sc;

public:
  explicit redis_client(const std::string& host, starcry& sc);
  ~redis_client();

  void run(bitmap_wrapper& bitmap, rendering_engine& engine);
};