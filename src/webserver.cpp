/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "webserver.h"

#include "starcry.h"

#include <memory>

Handler::Handler(starcry *sc) : sc(sc) {}

void Handler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void Handler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
}

void Handler::onData(seasocks::WebSocket *con, const char *data) {
  sc->add_command(con, "input/test.js", std::atoi(data));
}

void Handler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send((const uint8_t *)s.c_str(), s.size() * sizeof(uint8_t));
  }
}

std::shared_ptr<seasocks::Response> DataHandler::handle(const seasocks::CrackedUri & /*uri*/,
                                                        const seasocks::Request &request) {
  return seasocks::Response::jsonResponse("{}");
}

webserver::webserver(starcry *sc)
    : server(std::make_shared<seasocks::Server>(
          std::make_shared<seasocks::PrintfLogger>(seasocks::PrintfLogger::Level::Info))),
      image_handler(std::make_shared<Handler>(sc)) {
  auto root = std::make_shared<seasocks::RootPageHandler>();
  root->add(std::make_shared<seasocks::PathHandler>("data", std::make_shared<DataHandler>()));
  server->addPageHandler(root);
  server->addWebSocketHandler("/image", image_handler);
};

void webserver::run() {
  server->serve("webroot", 18080);
}
