/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "webserver.h"

#include "starcry.h"
#include "stats.h"  // piper

#include <memory>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

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
      script_handler(std::make_shared<ScriptHandler>(sc)),
      stats_handler(std::make_shared<StatsHandler>(sc)),
      objects_handler(std::make_shared<ObjectsHandler>(sc)),
      viewpoint_handler(std::make_shared<ViewPointHandler>(sc)) {
  auto root = std::make_shared<seasocks::RootPageHandler>();
  root->add(std::make_shared<seasocks::PathHandler>("data", std::make_shared<DataHandler>()));
  server->addPageHandler(root);
  server->setClientBufferSize(6653847300);
  server->addWebSocketHandler("/image", image_handler);
  server->addWebSocketHandler("/shapes", shapes_handler);
  server->addWebSocketHandler("/bitmap", bitmap_handler);
  server->addWebSocketHandler("/script", script_handler);
  server->addWebSocketHandler("/stats", stats_handler);
  server->addWebSocketHandler("/objects", objects_handler);
  server->addWebSocketHandler("/viewpoint", viewpoint_handler);
};

void webserver::run() {
  server->serve("webroot", 18080);
}

void webserver::send_stats(const stats &stats_) {
  json result = {};
  std::string str;
  const auto stats_map = stats_.get_raw();

  for (const auto &it : stats_map) {
    const auto s = it.first;
    const auto node_stats = it.second;
    json j = {
        {"name", node_stats.name},
        {"counter", node_stats.counter},
    };
    result.push_back(j);
  }
  str = result.dump();
  for (const auto &con : stats_handler->_cons) {
    if (server) {
      server->execute([=]() {
        con->send(str);
      });
    }
  }
}
