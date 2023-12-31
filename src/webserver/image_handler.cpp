/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "data/frame_request.hpp"
#include "starcry.h"
#include "util/logger.h"
#include "util/threadname.hpp"
#include "webserver.h"

ImageHandler::ImageHandler(starcry* sc) : sc(sc) {
  set_thread_name("ImageHandler");
}

void ImageHandler::onConnect(seasocks::WebSocket* con) {
  _cons.insert(con);
}

void ImageHandler::onDisconnect(seasocks::WebSocket* con) {
  _cons.erase(con);
  unlink(con);
}

void ImageHandler::onData(seasocks::WebSocket* con, const char* data) {
  std::string input(data);
  if (link(input, con)) return;
  auto find = input.find(" ");
  if (find != std::string::npos) {
    logger(DEBUG) << "ImageHandler::onData - " << input << std::endl;
    const auto script = input.substr(0, find);
    const auto frame_num = std::atoi(input.substr(find + 1).c_str());
    auto req = std::make_shared<data::frame_request>(script, frame_num, 1);
    req->set_websocket(con);
    req->enable_compressed_image();
    sc->add_image_command(req);
  }
}

void ImageHandler::callback(seasocks::WebSocket* recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send((const uint8_t*)s.c_str(), s.size() * sizeof(uint8_t));
  }
}
