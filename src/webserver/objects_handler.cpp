/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "data/frame_request.hpp"
#include "starcry.h"
#include "util/logger.h"
#include "webserver.h"

ObjectsHandler::ObjectsHandler(starcry* sc) : sc(sc) {}

void ObjectsHandler::onConnect(seasocks::WebSocket* con) {
  _cons.insert(con);
}

void ObjectsHandler::onDisconnect(seasocks::WebSocket* con) {
  _cons.erase(con);
  unlink(con);
}

void ObjectsHandler::onData(seasocks::WebSocket* con, const char* data) {
  std::string input(data);
  if (link(input, con)) return;

  const auto first_space = input.find(" ");
  if (first_space != std::string::npos) {
    logger(DEBUG) << "ObjectsHandler::onData - " << input << std::endl;
    const auto script = input.substr(0, first_space);

    const auto second_space = input.find(" ", first_space + 1);
    const auto frame_num = std::atoi(input.substr(first_space + 1, second_space - first_space - 1).c_str());
    const auto timeout = std::atoi(input.substr(second_space + 1).c_str());

    auto req = std::make_shared<data::frame_request>(script, frame_num, 1, timeout);
    req->set_websocket(con);
    req->enable_metadata_objects();
    sc->add_image_command(req);
  }
}

void ObjectsHandler::callback(seasocks::WebSocket* recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    // text/json
    recipient->send(s);
    // binary (we might need this soon if we attempt again)
    // recipient->send((const uint8_t*)s.c_str(), s.size() * sizeof(uint8_t));
  }
}

void callback_to_objects_handler(std::shared_ptr<ObjectsHandler> objects_handler,
                                 std::shared_ptr<render_msg> job_msg,
                                 seasocks::WebSocket* job_client,
                                 uint32_t width,
                                 uint32_t height) {
  if (objects_handler->_links.contains(job_msg->ID)) {
    auto con = objects_handler->_links[job_msg->ID];  // find con that matches ID this msg is from
    objects_handler->callback(con, job_msg->buffer);
  }
};
