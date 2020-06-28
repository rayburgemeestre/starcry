/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>
#include <functional>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "util/socketbuffer.h"
#include "data/job.hpp"
#include "data/pixels.hpp"

#define PORT "10000" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
extern void *get_in_addr(struct sockaddr *sa);

class render_client
{
private:
  int sockfd, numbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];
  socketbuffer buffer;
  std::vector<std::pair<int, std::string>> messages;
  std::function<void(int fd, int type, size_t len, const std::string& msg)> msg_callback;

public:
  render_client();
  ~render_client();

  void poll(std::function<void(int fd, int type, size_t len, const std::string& msg)> fn);
  void process();

  void register_me();
  void pull_job(bool is_remote, int64_t timestamp);
  void send_frame(const data::job &job, const data::pixel_data2 &dat, bool is_remote);
};
