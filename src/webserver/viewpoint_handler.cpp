/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "starcry.h"
#include "webserver.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "data/viewpoint.hpp"

#include <sstream>

ViewPointHandler::ViewPointHandler(starcry *sc) : sc(sc) {}

void ViewPointHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void ViewPointHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
}

void ViewPointHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  auto json = nlohmann::json::parse(input);

  if (json["operation"] == "read") {
    nlohmann::json response;
    const auto &vp = sc->get_viewpoint();
    response["scale"] = vp.scale;
    response["offset_x"] = vp.offset_x;
    response["offset_y"] = vp.offset_y;
    response["raw"] = vp.raw;
    response["preview"] = vp.preview;
    con->send(response.dump());
  } else if (json["operation"] == "set") {
    data::viewpoint vp{json["scale"], json["offset_x"], json["offset_y"], json["raw"], json["preview"]};
    sc->set_viewpoint(vp);
  }
}

void ViewPointHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(s);
  }
}
