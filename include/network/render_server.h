/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "data/job.hpp"
#include "util/socketbuffer.h"

class queue;

class render_server {
private:
  std::atomic<bool> running = true;

  // common
  struct pollfd *pfds = nullptr;
  int listener;                        // Listening socket descriptor
  int newfd;                           // newly accept()ed socket descriptor
  struct sockaddr_storage remoteaddr;  // client address
  socklen_t addrlen;
  char buf[51200];  // buffer for client data
  char remoteIP[INET6_ADDRSTRLEN];

  std::map<int, socketbuffer> buffers;
  std::mutex buffers_mut;
  std::function<bool(int fd, int type, size_t len, const std::string &msg)> msg_callback;
  std::map<int, socketbuffer> send_buffers;

  // poll based stuff
  // Start off with room for 5 connections
  // (We'll realloc as necessary)
  int fd_count = 0;
  int fd_size = 5;

  std::shared_ptr<queue> source;
  std::shared_ptr<queue> dest;
  std::thread runner;

public:
  render_server(std::shared_ptr<queue> source, std::shared_ptr<queue> dest);
  ~render_server();

  void run(std::function<bool(int fd, int type, size_t len, const std::string &msg)> fn);

  bool process(int fd);

  void poll_and_send();

  int send_msg(int fd, int type, const char *data, int len_data);

  void send_job(int fd, bool to_files, int64_t timestamp, const data::job job);
};
