/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdexcept>

#include "render_client.h"

#include "util/socket_utils.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

render_client::render_client()
{
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // TODO: do not hard-code this stuff
  if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
    //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    //return 1;
    throw std::runtime_error("getaddrinfo");
  }

  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    throw std::runtime_error("client: failed to connect");
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // all done with this structure
}

render_client::~render_client() {
  close(sockfd);
}

void render_client::poll(std::function<void(int fd, int type, size_t len, const std::string& msg)> fn) {
  msg_callback = fn;
  auto n = recvtimeout(sockfd, buf, sizeof buf, 0);
  if (n == -1) {
    // error occurred
    perror("recvtimeout");
  }
  else if (n == -2) {
    // timeout occurred
  } else {
    buffer.append(buf, n);
  }
  process();
}

void render_client::process()
{
  const std::string &buf = buffer.get();
  const char *raw = buf.c_str();
  const size_t buf_len = buffer.length();

  if (buf_len >= sizeof(int) * 2) {
    int *tmp = (int *)raw;
    int type = *tmp;
    int len = *(tmp + 1);
    if (len + (sizeof(int)*2) <= buf_len) {
      messages.emplace_back(std::make_pair(type, buf.substr(sizeof(int) * 2, len)));
      msg_callback(sockfd, type, size_t(len), buf.substr(sizeof(int) * 2, len));
      buffer.erase_front((sizeof(int) * 2) + len);
    }
  }
}

void render_client::register_me()
{
  send_msg(sockfd, 10, "", 0);
}

void render_client::pull_job(bool is_remote, int64_t timestamp)
{
  size_t len = sizeof(is_remote) + sizeof(timestamp);
  char *msg = (char *)malloc(len * sizeof(char));
  bool *p = ((bool *)msg);
  *p = is_remote;
  p++;
  int64_t *p2 = (int64_t *)p;
  *p2 = timestamp;
  send_msg(sockfd, 20, msg, len);
}

void render_client::send_frame(const data::job &job, const data::pixel_data2 &dat, bool is_remote)
{
  std::ostringstream os;
  cereal::BinaryOutputArchive archive(os);
  archive(job);
  archive(dat);
  archive(is_remote);
  send_msg(sockfd, 30, os.str().c_str(), os.str().size());
}
