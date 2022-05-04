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
#include "util/logger.h"

ViewPointHandler::ViewPointHandler(starcry *sc) : sc(sc) {}

void ViewPointHandler::onConnect(seasocks::WebSocket *con) {
  _cons.insert(con);
}

void ViewPointHandler::onDisconnect(seasocks::WebSocket *con) {
  _cons.erase(con);
  unlink(con);
}

void ViewPointHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  if (link(input, con)) return;
  auto json = nlohmann::json::parse(input);
  logger(DEBUG) << "ViewPointHandler::onData - " << input << std::endl;
  if (json["operation"] == "read") {
    nlohmann::json response;
    const auto &vp = sc->get_viewpoint();
    response["scale"] = vp.scale;
    response["offset_x"] = vp.offset_x;
    response["offset_y"] = vp.offset_y;
    response["raw"] = vp.raw;
    response["preview"] = vp.preview;
    response["save"] = vp.save;
    response["labels"] = vp.labels;
    response["caching"] = vp.caching;
    response["debug"] = vp.debug;
    con->send(response.dump());
  } else if (json["operation"] == "set") {
    data::viewpoint vp{json["scale"],
                       json["offset_x"],
                       json["offset_y"],
                       json["raw"],
                       json["preview"],
                       json["save"],
                       json["labels"],
                       json["caching"],
                       json["debug"],
                       json["canvas_w"],
                       json["canvas_h"]};
    sc->set_viewpoint(vp);
  }
}

void ViewPointHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(s);
  }
}
