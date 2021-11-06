/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "webserver/starcry_handler.hpp"

struct ShapesHandler : seasocks::WebSocket::Handler, public starcry_handler {
  starcry *sc;
  std::set<seasocks::WebSocket *> _cons;

  explicit ShapesHandler(starcry *sc);

  void onConnect(seasocks::WebSocket *con) override;
  void onDisconnect(seasocks::WebSocket *con) override;
  void onData(seasocks::WebSocket *con, const char *data) override;

  void callback(seasocks::WebSocket *recipient, std::string s);
};
