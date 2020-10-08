/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "webserver.h"

#include "starcry.h"

#include <memory>
#include <sstream>

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
    sc->add_command(con, input.substr(0, find), instruction_type::get_image, std::atoi(input.substr(find + 1).c_str()));
  }
}

void ImageHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send((const uint8_t *)s.c_str(), s.size() * sizeof(uint8_t));
  }
}

ShapesHandler::ShapesHandler(starcry *sc) : sc(sc) {}

void ShapesHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void ShapesHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
}

void ShapesHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  auto find = input.find(" ");
  if (find != std::string::npos) {
    sc->add_command(
        con, input.substr(0, find), instruction_type::get_shapes, std::atoi(input.substr(find + 1).c_str()));
  }
}

void ShapesHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send((const uint8_t *)s.c_str(), s.size() * sizeof(uint8_t));
  }
}

BitmapHandler::BitmapHandler(starcry *sc) : sc(sc) {}

void BitmapHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void BitmapHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
}

void BitmapHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  auto find = input.find(" ");
  if (find != std::string::npos) {
    sc->add_command(
      con, input.substr(0, find), instruction_type::get_bitmap, std::atoi(input.substr(find + 1).c_str()));
  }
}

void BitmapHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send((const uint8_t *)s.c_str(), s.size() * sizeof(uint8_t));
  }
}

ScriptHandler::ScriptHandler(starcry *sc) : sc(sc) {}

void ScriptHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void ScriptHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
}

void ScriptHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::ifstream ifs(data);
  std::ostringstream ss;
  ss << ifs.rdbuf();
  con->send((const uint8_t *)ss.str().c_str(), ss.str().size() * sizeof(uint8_t));
}

void ScriptHandler::callback(seasocks::WebSocket *recipient, std::string s) {
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
      image_handler(std::make_shared<ImageHandler>(sc)),
      shapes_handler(std::make_shared<ShapesHandler>(sc)),
      bitmap_handler(std::make_shared<BitmapHandler>(sc)),
      script_handler(std::make_shared<ScriptHandler>(sc))
{
  auto root = std::make_shared<seasocks::RootPageHandler>();
  root->add(std::make_shared<seasocks::PathHandler>("data", std::make_shared<DataHandler>()));
  server->addPageHandler(root);
  server->setClientBufferSize(6653847300);
  server->addWebSocketHandler("/image", image_handler);
  server->addWebSocketHandler("/shapes", shapes_handler);
  server->addWebSocketHandler("/bitmap", bitmap_handler);
  server->addWebSocketHandler("/script", script_handler);
};

void webserver::run() {
  server->serve("webroot", 18080);
}
