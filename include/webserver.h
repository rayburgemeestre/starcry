/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "seasocks/PrintfLogger.h"
#include "seasocks/Response.h"
#include "seasocks/Server.h"
#include "seasocks/StringUtil.h"
#include "seasocks/WebSocket.h"
#include "seasocks/util/PathHandler.h"
#include "seasocks/util/RootPageHandler.h"

#include "messages.hpp"

class starcry;

struct ImageHandler : seasocks::WebSocket::Handler {
  starcry *sc;
  std::set<seasocks::WebSocket *> _cons;
  explicit ImageHandler(starcry *sc);
  void onConnect(seasocks::WebSocket *con) override;
  void onDisconnect(seasocks::WebSocket *con) override;
  void onData(seasocks::WebSocket *con, const char *data) override;
  void callback(seasocks::WebSocket *recipient, std::string s);
};

struct ShapesHandler : seasocks::WebSocket::Handler {
  starcry *sc;
  std::set<seasocks::WebSocket *> _cons;
  explicit ShapesHandler(starcry *sc);
  void onConnect(seasocks::WebSocket *con) override;
  void onDisconnect(seasocks::WebSocket *con) override;
  void onData(seasocks::WebSocket *con, const char *data) override;
  void callback(seasocks::WebSocket *recipient, std::string s);
};

struct BitmapHandler : seasocks::WebSocket::Handler {
  starcry *sc;
  std::set<seasocks::WebSocket *> _cons;
  explicit BitmapHandler(starcry *sc);
  void onConnect(seasocks::WebSocket *con) override;
  void onDisconnect(seasocks::WebSocket *con) override;
  void onData(seasocks::WebSocket *con, const char *data) override;
  void callback(seasocks::WebSocket *recipient, std::string s);
};

struct ScriptHandler : seasocks::WebSocket::Handler {
  starcry *sc;
  std::set<seasocks::WebSocket *> _cons;
  ScriptHandler(starcry *sc);
  void onConnect(seasocks::WebSocket *con) override;
  void onDisconnect(seasocks::WebSocket *con) override;
  void onData(seasocks::WebSocket *con, const char *data) override;
  void callback(seasocks::WebSocket *recipient, std::string s);
};

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

public:
  explicit webserver(starcry *sc);

  void run();

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

  void send_stats(const stats &s);
};