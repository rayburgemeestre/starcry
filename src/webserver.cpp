/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "webserver.h"

#include "starcry.h"
#include "stats.h"  // piper

#include <filesystem>

#include "nlohmann/json.hpp"
#include "webserver/script_handler.h"

using json = nlohmann::json;

std::shared_ptr<seasocks::Response> DataHandler::handle(const seasocks::CrackedUri & /*uri*/,
                                                        const seasocks::Request &request) {
  return seasocks::Response::jsonResponse("{}");
}

#include "seasocks/Logger.h"
#include "util/logger.h"

class MyLogger : public seasocks::Logger {
public:
  explicit MyLogger(Level minLevelToLog_ = Level::Debug) : minLevelToLog(minLevelToLog_) {}
  ~MyLogger() = default;
  virtual void log(Level level, const char *message) override {
    if (level >= minLevelToLog) {
      switch (level) {
        case Level::Access:
          logger(DEBUG) << "[access]" << message << std::endl;
          return;
        case Level::Debug:
          logger(DEBUG) << message << std::endl;
          return;
        case Level::Info:
          logger(INFO) << message << std::endl;
          return;
        case Level::Warning:
        case Level::Error:
          logger(WARNING) << message << std::endl;
          return;
        case Level::Severe:
          logger(FATAL) << message << std::endl;
          return;
      }
    }
  }
  Level minLevelToLog;
};

webserver::webserver(starcry *sc)
    : server(std::make_shared<seasocks::Server>(std::make_shared<MyLogger>(MyLogger::Level::Info))),
      bitmap_handler(std::make_shared<BitmapHandler>(sc)),
      script_handler(std::make_shared<ScriptHandler>(sc)),
      stats_handler(std::make_shared<StatsHandler>(sc)),
      objects_handler(std::make_shared<ObjectsHandler>(sc)),
      viewpoint_handler(std::make_shared<ViewPointHandler>(sc)) {
  auto root = std::make_shared<seasocks::RootPageHandler>();
  root->add(std::make_shared<seasocks::PathHandler>("data", std::make_shared<DataHandler>()));
  server->addPageHandler(root);
  server->setClientBufferSize(6653847300);
  server->addWebSocketHandler("/bitmap", bitmap_handler);
  server->addWebSocketHandler("/script", script_handler);
  server->addWebSocketHandler("/stats", stats_handler);
  server->addWebSocketHandler("/objects", objects_handler);
  server->addWebSocketHandler("/viewpoint", viewpoint_handler);
}

void webserver::set_script(const std::string &script) {
  script_handler->set_script(script);
}

void webserver::log_line(const std::string &log_line) {
  script_handler->log_line(log_line);
}

void webserver::run() {
  std::string str = "web/webroot";
  if (!std::filesystem::exists(str)) {
    str = "web/dist/spa";
  }
  if (!std::filesystem::exists(str)) {
    str = "/workdir/web/webroot";
  }
  server->serve(str.c_str(), 18080);
}

void webserver::stop() {
  server->terminate();
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
  str = json{{"type", "stats"}, {"data", result}}.dump();
  for (const auto &con : stats_handler->_cons) {
    if (server) {
      server->execute([=]() {
        con->send(str);
      });
    }
  }
}

void webserver::send_metrics(const std::string &json) {
  for (const auto &con : stats_handler->_cons) {
    if (server) {
      server->execute([=]() {
        con->send(json);
      });
    }
  }
}

void webserver::send_fs_change(const std::string &json) {
  for (const auto &con : stats_handler->_cons) {
    if (server) {
      server->execute([=]() {
        con->send(json);
      });
    }
  }
}
