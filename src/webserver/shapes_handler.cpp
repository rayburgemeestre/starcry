/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
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
    sc->add_command(con,
                    input.substr(0, find),
                    instruction_type::get_shapes,
                    std::atoi(input.substr(find + 1).c_str()),
                    1,
                    false,
                    false,
                    false,
                    "");
  }
}

void ShapesHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(s);
  }
}
