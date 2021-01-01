/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "starcry.h"
#include "webserver.h"

ImageHandler::ImageHandler(starcry *sc) : sc(sc) {}

void ImageHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void ImageHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
}

void ImageHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  auto find = input.find(" ");
  if (find != std::string::npos) {
    sc->add_command(
        con, input.substr(0, find), instruction_type::get_image, std::atoi(input.substr(find + 1).c_str()), 1);
  }
}

void ImageHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send((const uint8_t *)s.c_str(), s.size() * sizeof(uint8_t));
  }
}
