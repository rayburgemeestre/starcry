/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "starcry.h"
#include "webserver.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <filesystem>
#include <sstream>

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
    json result = {};
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
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
      json j = {
          {"filename", data},
          {"filesize", file_size},
          {"modified", time.substr(0, time.size() - 1)},
      };
      result.push_back(j);
    }
    con->send(result.dump());
  }
}

void ScriptHandler::callback(seasocks::WebSocket *recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(s);
  }
}