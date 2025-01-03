/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "webserver/starcry_handler.hpp"

struct BitmapHandler : seasocks::WebSocket::Handler, public starcry_handler {
  starcry *sc;
  std::set<seasocks::WebSocket *> _cons;

  explicit BitmapHandler(starcry *sc);

  void onConnect(seasocks::WebSocket *con) override;
  void onDisconnect(seasocks::WebSocket *con) override;
  void onData(seasocks::WebSocket *con, const char *data) override;

  void callback(seasocks::WebSocket *recipient,
                std::string s,
                uint32_t width,
                uint32_t height,
                uint32_t chunk,
                uint32_t num_chunks);
  void callback(seasocks::WebSocket *recipient);
};
void callback_to_bmp_handler(std::shared_ptr<BitmapHandler> bmp_handler,
                             std::shared_ptr<render_msg> job_msg,
                             seasocks::WebSocket *job_client,
                             uint32_t width,
                             uint32_t height,
                             uint32_t chunk,
                             uint32_t num_chunks);
