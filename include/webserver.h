/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <thread>
#include "crow.h"

struct content_type_fixer {
  struct context {};
  void before_handle(crow::request& req, crow::response& res, context& ctx);
  void after_handle(crow::request& req, crow::response& res, context& ctx);
};

class webserver {
private:
  crow::App<content_type_fixer> app;
  std::thread webserver_;

public:
  webserver();
  ~webserver();

  void start();
  void stop();
};