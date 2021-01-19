#pragma once

#include <string>

class starcry;

class client_message_handler {
private:
  starcry &sc;
  size_t outstanding_jobs = 0;
  bool send_last = false;
  bool recv_last = false;

public:
  client_message_handler(starcry &sc);

  bool on_client_message(int sockfd, int type, size_t len, const std::string &data);
};
