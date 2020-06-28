/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <functional>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include "util/socketbuffer.h"

class render_server
{
private:
  int listener;     // Listening socket descriptor

  int newfd;        // Newly accept()ed socket descriptor
  struct sockaddr_storage remoteaddr; // Client address
  socklen_t addrlen;

  char buf[256];    // Buffer for client data

  char remoteIP[INET6_ADDRSTRLEN];

  // Start off with room for 5 connections
  // (We'll realloc as necessary)
  int fd_count = 0;
  int fd_size = 5;

  struct pollfd *pfds = nullptr;
  std::map<int, socketbuffer> buffers;
  std::map<int, std::vector<std::pair<int, std::string>>> messages;
  std::mutex messages_mut;
  std::function<void(int fd, int type, size_t len, const std::string& msg)> msg_callback;

public:

  render_server();

  void run(std::function<void(int fd, int type, size_t len, const std::string &msg)> fn);

  void process(int fd);

  ~render_server();
};
