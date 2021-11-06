/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "seasocks/Response.h"
#include "seasocks/Server.h"
#include "seasocks/StringUtil.h"
#include "seasocks/WebSocket.h"
#include "seasocks/util/PathHandler.h"
#include "seasocks/util/RootPageHandler.h"

#include "messages.hpp"

class starcry;

#include "webserver/bitmap_handler.h"
#include "webserver/image_handler.h"
#include "webserver/objects_handler.h"
#include "webserver/script_handler.h"
#include "webserver/shapes_handler.h"
#include "webserver/stats_handler.h"
#include "webserver/viewpoint_handler.h"

struct DataHandler : seasocks::CrackedUriPageHandler {
  virtual std::shared_ptr<seasocks::Response> handle(const seasocks::CrackedUri & /*uri*/,
                                                     const seasocks::Request &request) override;
};

class stats;

class webserver {
private:
  std::shared_ptr<seasocks::Server> server;
  std::shared_ptr<ImageHandler> image_handler;
  std::shared_ptr<ShapesHandler> shapes_handler;
  std::shared_ptr<BitmapHandler> bitmap_handler;
  std::shared_ptr<ScriptHandler> script_handler;
  std::shared_ptr<StatsHandler> stats_handler;
  std::shared_ptr<ObjectsHandler> objects_handler;
  std::shared_ptr<ViewPointHandler> viewpoint_handler;

public:
  explicit webserver(starcry *sc);

  void run();

  void set_script(const std::string &script);

  template <typename T>
  void execute_image(T fun, std::shared_ptr<render_msg> job_msg) {
    if (server) {
      server->execute(std::bind(fun, image_handler, job_msg));
    }
  }
  template <typename T>
  void execute_shapes(T fun, std::shared_ptr<render_msg> job_msg) {
    if (server) {
      server->execute(std::bind(fun, shapes_handler, job_msg));
    }
  }
  template <typename T>
  void execute_bitmap(T fun, std::shared_ptr<render_msg> job_msg) {
    if (server) {
      server->execute(std::bind(fun, bitmap_handler, job_msg));
    }
  }
  template <typename T>
  void execute_objects(T fun, std::shared_ptr<render_msg> job_msg) {
    if (server) {
      server->execute(std::bind(fun, objects_handler, job_msg));
    }
  }

  std::string get_client_id(seasocks::WebSocket *con) {
    if (server) {
      for (const auto &link : bitmap_handler->_links) {
        if (link.second == con) {
          return link.first;
        }
      }
    }
    return "";
  }

  void send_stats(const stats &s);
};
