/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "starcry.h"
#include "webserver.h"

#include "data/frame_request.hpp"
#include "data/video_request.hpp"
#include "nlohmann/json.hpp"
#include "util/logger.h"
#include "util/threadname.hpp"

using json = nlohmann::json;

BitmapHandler::BitmapHandler(starcry* sc) : sc(sc) {
  set_thread_name("BitmapHandler");
}

void BitmapHandler::onConnect(seasocks::WebSocket* con) {
  _cons.insert(con);
}

void BitmapHandler::onDisconnect(seasocks::WebSocket* con) {
  _cons.erase(con);
  unlink(con);
}

void BitmapHandler::onData(seasocks::WebSocket* con, const char* data) {
  std::string input(data);
  if (link(input, con)) return;
  logger(DEBUG) << "BitmapHandler::onData - " << input << std::endl;
  auto json = nlohmann::json::parse(input);
  auto num_chunks = 32;
  std::vector<int64_t> selected_ids;
  if (json["num_chunks"].is_number_integer()) {
    num_chunks = json["num_chunks"];
  }
  if (json["selected"].is_array()) {
    selected_ids = json["selected"].get<std::vector<int64_t>>();
  }
  if (json["filename"].is_string() && json["frame"].is_number_integer()) {
    if (!json["video"].is_boolean() || !bool(json["video"])) {
      auto req = std::make_shared<data::frame_request>(json["filename"], json["frame"], num_chunks);
      req->set_websocket(con);
      req->enable_raw_bitmap();
      req->set_selected_ids(selected_ids);
      if (sc->get_viewpoint().raw) {
        req->enable_raw_image();
      }
      if (sc->get_viewpoint().preview) {
        req->set_preview_mode();
      }
      if (sc->get_viewpoint().labels) {
        req->enable_metadata_objects();
      }
      sc->add_image_command(req);
    } else {
      const int frame = size_t(json["frame"]);
      const int width = json["width"].is_number_integer() ? int(json["width"]) : 1920;
      const int height = json["height"].is_number_integer() ? int(json["height"]) : 1080;
      auto req = std::make_shared<data::video_request>(
          json["filename"], sc->options().output_file, num_chunks, frame, width, height);
      req->enable_compressed_video();
      req->set_websocket(con);
      // req->set_selected_ids(selected_ids);
      if (sc->get_viewpoint().raw) {
        // req->enable_raw_video();
      }
      if (sc->get_viewpoint().preview) {
        req->set_preview_mode();
      }
      if (sc->get_viewpoint().labels) {
        // req->enable_metadata_objects();
      }
      sc->add_video_command(req);
    }
  }
}

void BitmapHandler::callback(seasocks::WebSocket* recipient, std::string s, uint32_t width, uint32_t height) {
  if (_cons.find(recipient) != _cons.end()) {
    size_t n = s.size();
    // append 8 chars (room for 2 32-bit ints)
    s.append("        ");
    char* ptr = s.data() + n;
    memcpy(ptr, &width, sizeof(width));
    memcpy(ptr + sizeof(uint32_t), &height, sizeof(height));
    recipient->send((const uint8_t*)s.c_str(), s.size() * sizeof(uint8_t));
  }
}

void BitmapHandler::callback(seasocks::WebSocket* recipient) {
  if (_cons.find(recipient) != _cons.end()) {
    recipient->send(nullptr, 0);
  }
}
