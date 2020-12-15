/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "starcry.h"
#include "webserver.h"

ObjectsHandler::ObjectsHandler(starcry *sc) : sc(sc) {}

void ObjectsHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void ObjectsHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
}

void ObjectsHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  auto find = input.find(" ");
  if (find != std::string::npos) {
    sc->add_command(
        con, input.substr(0, find), instruction_type::get_objects, std::atoi(input.substr(find + 1).c_str()));
  }
}

void ObjectsHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(s);
  }
}
