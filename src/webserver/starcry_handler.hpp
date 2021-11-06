/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

class starcry_handler {
public:
  std::unordered_map<std::string, seasocks::WebSocket *> _links;

public:
  virtual ~starcry_handler() = default;

  bool link(const std::string &input, seasocks::WebSocket *con) {
    if (input.size() > 5 && input.substr(0, 4) == "LINK") {
      _links[input.substr(5)] = con;
      return true;
    }
    return false;
  }

  void unlink(seasocks::WebSocket *con) {
    for (const auto &link : _links) {
      if (link.second == con) {
        _links.erase(link.first);
        return;
      }
    }
  }
};