/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "data/frame_request.hpp"
#include "starcry.h"
#include "util/logger.h"
#include "webserver.h"

ObjectsHandler::ObjectsHandler(starcry *sc) : sc(sc) {}

void ObjectsHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void ObjectsHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
  unlink(con);
}

void ObjectsHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  if (link(input, con)) return;
  auto find = input.find(" ");
  if (find != std::string::npos) {
    logger(DEBUG) << "ObjectsHandler::onData - " << input << std::endl;
    const auto script = input.substr(0, find);
    const auto frame_num = std::atoi(input.substr(find + 1).c_str());
    auto req = std::make_shared<data::frame_request>(script, frame_num, 1);
    req->set_websocket(con);
    req->enable_metadata_objects();
    sc->add_image_command(req);
  }
}

void ObjectsHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(s);
  }
}
