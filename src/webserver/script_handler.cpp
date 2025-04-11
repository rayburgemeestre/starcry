/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "specs/video.hpp"
#include "starcry.h"
#include "util/logger.h"
#include "webserver.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <filesystem>
#include <sstream>

ScriptHandler::ScriptHandler(starcry* sc) : sc(sc) {}

void ScriptHandler::set_script(const std::string& script) {
  script_ = script;
}

void ScriptHandler::onConnect(seasocks::WebSocket* con) {
  _cons.insert(con);
  std::ostringstream ss;
  ss << "1" << script_;
  con->send(ss.str());
}

void ScriptHandler::onDisconnect(seasocks::WebSocket* con) {
  _cons.erase(con);
  unlink(con);
}

// Source https://stackoverflow.com/a/61067330
// Get rid of this once we are at C++20
template <typename TP>
std::time_t to_time_t(TP tp) {
  using namespace std::chrono;
  auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
  return system_clock::to_time_t(sctp);
}

void ScriptHandler::onData(seasocks::WebSocket* con, const char* data) {
  std::string input(data);
  if (link(input, con)) return;
  const auto find = input.find(" ");
  if (find != std::string::npos) {
    const auto cmd = input.substr(0, find);
    if (cmd == "open") {
      const auto file = input.substr(find + 1);
      set_script(file);
      logger(DEBUG) << "ScriptHandler::onData - " << input << std::endl;
      std::ifstream ifs(file);
      std::ostringstream ss;
      // send JS api first (for blending_type definitions, etc.)
      ss << "6" << sc->get_js_api();
      con->send(ss.str());
      ss.str("");
      ss.clear();
      // send the script after that
      ss << "2" << ifs.rdbuf();
      con->send(ss.str());
    } else if (cmd == "set") {
      const auto file_contents = input.substr(find + 1);
      logger(DEBUG) << "ScriptHandler::onData - " << input << std::endl;
      sc->update_script_contents(file_contents);
      std::ostringstream ss;
      // TODO: we can re-use 3 for this, and rename all the others... 1 and 3 are the same.
      ss << "3" << sc->script();
      con->send(ss.str());
    }
  } else if (input == "list") {
    logger(DEBUG) << "ScriptHandler::onData - " << input << std::endl;
    json result = {};
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
    for (const auto& entry : recursive_directory_iterator("input")) {
      if (!entry.is_regular_file()) continue;
      const auto data = std::string(entry.path());
      const auto file_size = std::to_string(entry.file_size());
      std::stringstream file_size_ss;     // workaround for compiler issue??
      file_size_ss << entry.file_size();  // EDIT: only clang has an issue
      auto timeEntry = std::filesystem::last_write_time(entry);
      time_t cftime = to_time_t(timeEntry);
      const auto time = std::string(std::asctime(std::localtime(&cftime)));
      const auto suffix = std::string(".js");
      const auto endswidth_js_ext = data.find(suffix, data.size() - suffix.size()) != std::string::npos;
      if (!endswidth_js_ext) continue;
      json j = {
          {"filename", data},
          {"filesize", file_size_ss.str()},
          {"modified", time.substr(0, time.size() - 1)},
      };
      result.push_back(j);
    }
    std::stringstream ss;
    ss << "4" << result.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);

    con->send(ss.str());
  } else if (input == "terminate") {
    logger(DEBUG) << "ScriptHandler::onData - " << input << std::endl;
    con->send("5");
    std::exit(1);
  } else if (input == "get_video_spec") {
    std::stringstream ss;
    const auto video_spec = this->sc->get_spec("video");
    ss << "7" << video_spec;
    con->send(ss.str());
  } else if (input == "get_object_spec") {
    std::stringstream ss;
    const auto object_spec = this->sc->get_spec("object");
    ss << "8" << object_spec;
    con->send(ss.str());
  }
}

void ScriptHandler::callback(seasocks::WebSocket* recipient, std::string s) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(s);
  }
}
