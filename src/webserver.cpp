/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "webserver.h"

#include "starcry.h"
#include "stats.h"  // piper

#include <filesystem>
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

// Source https://stackoverflow.com/a/61067330
// Get rid of this once we are at C++20
template <typename TP>
std::time_t to_time_t(TP tp) {
  using namespace std::chrono;
  auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
  return system_clock::to_time_t(sctp);
}

void ScriptHandler::onData(seasocks::WebSocket *con, const char *data) {
  std::string input(data);
  const auto find = input.find(" ");
  if (find != std::string::npos) {
    const auto cmd = input.substr(0, find);
    const auto file = input.substr(find + 1);
    if (cmd == "open") {
      std::ifstream ifs(file);
      std::ostringstream ss;
      ss << ifs.rdbuf();
      con->send((const uint8_t *)ss.str().c_str(), ss.str().size() * sizeof(uint8_t));
    }
  } else if (input == "list") {
    std::stringstream ss;
    ss << "[";
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
    bool first = true;
    for (const auto &entry : recursive_directory_iterator("input")) {
      if (!entry.is_regular_file()) continue;
      const auto data = std::string(entry.path());
      const auto file_size = std::to_string(entry.file_size());
      auto timeEntry = std::filesystem::last_write_time(entry);
      time_t cftime = to_time_t(timeEntry);
      const auto time = std::string(std::asctime(std::localtime(&cftime)));
      const auto suffix = std::string(".js");
      const auto endswidth_js_ext = data.find(suffix, data.size() - suffix.size()) != std::string::npos;
      if (!endswidth_js_ext) continue;
      if (!first) {
        ss << ",";
      }
      ss << "{\"filename\": " << entry << ", \"filesize\": \"" << file_size << "\", \"modified\": \""
         << time.substr(0, time.size() - 1) << "\"}" << std::endl;
      first = false;
    }
    ss << "]";
    con->send((const uint8_t *)ss.str().c_str(), ss.str().size() * sizeof(uint8_t));
  }
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
      script_handler(std::make_shared<ScriptHandler>(sc)) {
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

void webserver::send_stats(const stats &stats_) {
  std::stringstream ss;
  const auto stats_map = stats_.get_raw();
  for (const auto it : stats_map) {
    const auto s = it.first;
    const auto node_stats = it.second;
    ss << "first: " << s << std::endl;
    ss << "name: " << node_stats.name << std::endl;
    ss << "is_storage: " << std::boolalpha << node_stats.is_storage << std::endl;
    ss << "is_sleeping_until_not_full: " << std::boolalpha << node_stats.is_sleeping_until_not_full << std::endl;
    ss << "is_sleeping_until_not_empty: " << std::boolalpha << node_stats.is_sleeping_until_not_empty << std::endl;
    ss << "size: " << std::boolalpha << node_stats.size << std::endl;
    ss << "counter: " << std::boolalpha << node_stats.counter << std::endl;
    ss << "last_counter: " << std::boolalpha << node_stats.last_counter << std::endl;
  }
  for (const auto &con : script_handler->_cons) {
    if (server) {
      const auto str = ss.str();
      server->execute([=]() {
        con->send((const uint8_t *)str.c_str(), str.size() * sizeof(uint8_t));
      });
    }
  }
}
