#pragma once

#include <string>

class starcry;

class client_message_handler {
private:
  starcry &sc;

public:
  client_message_handler(starcry &sc);

  bool on_client_message(int sockfd, int type, size_t len, const std::string &data);
};
