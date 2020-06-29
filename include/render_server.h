/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <string>
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

class render_server {
private:
  enum class server_type { poll_based, select_based } type;

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
  std::function<void(int fd, int type, size_t len, const std::string &msg)> msg_callback;
  std::map<int, socketbuffer> send_buffers;

  // poll based stuff
  // Start off with room for 5 connections
  // (We'll realloc as necessary)
  int fd_count = 0;
  int fd_size = 5;

  // select based
  fd_set master;    // master file descriptor list
  fd_set read_fds;  // temp file descriptor list for select()
  int fdmax;        // maximum file descriptor number
  int nbytes;
  int yes = 1;  // for setsockopt() SO_REUSEADDR, below
  int i, rv;
  struct addrinfo hints, *ai, *p;

public:
  render_server();

  void run(std::function<void(int fd, int type, size_t len, const std::string &msg)> fn);

  void process(int fd);

  void poll_and_send();

  ~render_server();

  int send_msg(int fd, int type, const char *data, int len_data);

  void send_job(int fd, bool to_files, int64_t timestamp, const data::job job);
};
