/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "starcry.h"
#include "webserver.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

BitmapHandler::BitmapHandler(starcry *sc) : sc(sc) {}

void BitmapHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void BitmapHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
}

void BitmapHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  auto json = nlohmann::json::parse(input);
  sc->add_command(con,
                  json["filename"],
                  instruction_type::get_bitmap,
                  json["frame"],
                  1,
                  sc->get_viewpoint().raw,
                  sc->get_viewpoint().preview);
}

void BitmapHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send((const uint8_t *)s.c_str(), s.size() * sizeof(uint8_t));
  }
}
