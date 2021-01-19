#pragma once

#include <string>

class starcry;
class render_client;
class rendering_engine_wrapper;
class bitmap_wrapper;

class server_message_handler {
private:
  starcry &sc;

public:
  server_message_handler(starcry &sc);

  bool on_server_message(render_client &client,
                         rendering_engine_wrapper &engine,
                         bitmap_wrapper &bitmap,
                         int sockfd,
                         int type,
                         size_t len,
                         const std::string &data);
};
