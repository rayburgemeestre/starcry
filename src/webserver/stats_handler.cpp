/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "starcry.h"
#include "webserver.h"

StatsHandler::StatsHandler(starcry* sc) : sc(sc) {}

void StatsHandler::onConnect(seasocks::WebSocket* con) {
  _cons.insert(con);
}

void StatsHandler::onDisconnect(seasocks::WebSocket* con) {
  _cons.erase(con);
  unlink(con);
}

void StatsHandler::onData(seasocks::WebSocket* con, const char* data) {
  std::string input(data);
  if (link(input, con)) return;
}
