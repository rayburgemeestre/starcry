#pragma once

#include <map>
#include <memory>
#include <string>

class starcry;
class job_message;

class client_message_handler {
private:
  starcry &sc;
  size_t outstanding_jobs = 0;
  bool send_last = false;
  bool recv_last = false;

  // TODO: these make outstanding_jobs variable redundant
  using job_id_t = std::pair<uint32_t, uint32_t>;
  std::map<job_id_t, std::shared_ptr<job_message>> outstanding_jobs2;

public:
  client_message_handler(starcry &sc);

  bool on_client_message(int sockfd, int type, size_t len, const std::string &data);
};
