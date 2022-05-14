/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "data/frame_request.hpp"
#include "starcry.h"
#include "util/logger.h"
#include "webserver.h"

ShapesHandler::ShapesHandler(starcry *sc) : sc(sc) {}

void ShapesHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void ShapesHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
  unlink(con);
}

void ShapesHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  if (link(input, con)) return;
  auto find = input.find(" ");
  if (find != std::string::npos) {
    logger(DEBUG) << "ShapesHandler::onData - " << input << std::endl;
    // logger(INFO) << "ShapesHandler::onData received script: " << input.substr(0, find) << " get shapes for: " <<
    // std::atoi(input.substr(find + 1).c_str()) << std::endl; logger(INFO) << "ShapesHandler::onData recv: " << input
    // << std::endl;
    const auto script = input.substr(0, find);
    const auto frame_num = std::atoi(input.substr(find + 1).c_str());
    auto req = std::make_shared<data::frame_request>(script, frame_num, 1);
    req->set_websocket(con);
    req->enable_renderable_shapes();
  }
}

void ShapesHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(s);
  }
}
