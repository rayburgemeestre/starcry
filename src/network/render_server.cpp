/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <sstream>

#include "cereal/archives/binary.hpp"
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif  // __clang__
#include "cereal/archives/json.hpp"

#include "network/render_server.h"
#include "network/messages.h"

#define PORT "10000"

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void) {
  int listener;  // Listening socket descriptor
  int yes = 1;   // For setsockopt() SO_REUSEADDR, below
  int rv;

  struct addrinfo hints, *ai, *p;

  // Get us a socket and bind it
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
    exit(1);
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      continue;
    }

    // Lose the pesky "address already in use" error message
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener);
      continue;
    }

    break;
  }

  // If we got here, it means we didn't get bound
  if (p == NULL) {
    return -1;
  }

  freeaddrinfo(ai);  // All done with this

  // Listen
  if (listen(listener, 10) == -1) {
    return -1;
  }

  return listener;
}

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size) {
  // If we don't have room, add more space in the pfds array
  if (*fd_count == *fd_size) {
    *fd_size *= 2;  // Double it

    *pfds = (struct pollfd *)realloc(*pfds, sizeof(**pfds) * (*fd_size));
  }

  (*pfds)[*fd_count].fd = newfd;
  (*pfds)[*fd_count].events = POLLIN;  // Check ready-to-read

  (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
  // Copy the one from the end over this one
  pfds[i] = pfds[*fd_count - 1];

  (*fd_count)--;
}

render_server::render_server(std::shared_ptr<queue> source, std::shared_ptr<queue> dest) : source(source), dest(dest) {
  pfds = (struct pollfd *)malloc(sizeof *pfds * fd_size);

  // Set up and get a listening socket
  listener = get_listener_socket();

  if (listener == -1) {
    fprintf(stderr, "error getting listening socket\n");
    exit(1);
  }

  // Add the listener to set
  pfds[0].fd = listener;
  pfds[0].events = POLLIN;  // Report ready to read on incoming connection

  fd_count = 1;  // For the listener
}

render_server::~render_server() {
  runner.join();
}

void render_server::run(std::function<bool(int fd, int type, size_t len, const std::string &msg)> fn) {
  msg_callback = fn;
  runner = std::thread([&]() {
    while (true) {
      int poll_count = poll(pfds, fd_count, -1);

      if (poll_count == -1) {
        perror("poll");
        exit(1);
      }

      // Run through the existing connections looking for data to read
      for (int i = 0; i < fd_count; i++) {
        // Check if someone's ready to read
        if (pfds[i].revents & POLLIN) {  // We got one!!

          if (pfds[i].fd == listener) {
            // If listener is ready to read, handle new connection

            addrlen = sizeof remoteaddr;
            newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

            if (newfd == -1) {
              perror("accept");
            } else {
              add_to_pfds(&pfds, newfd, &fd_count, &fd_size);

              printf(
                  "pollserver: new connection from %s on "
                  "socket %d\n",
                  inet_ntop(
                      remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET6_ADDRSTRLEN),
                  newfd);
            }
          } else {
            // If not the listener, we're just a regular client
            int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

            int sender_fd = pfds[i].fd;

            if (nbytes <= 0) {
              // Got error or connection closed by client
              if (nbytes == 0) {
                // Connection closed
                printf("pollserver: socket %d hung up\n", sender_fd);
              } else {
                perror("recv");
              }

              close(pfds[i].fd);  // Bye!

              del_from_pfds(pfds, i, &fd_count);

            } else {
              // We got some good data from a client
              buffers[sender_fd].append(buf, nbytes);
              if (!process(sender_fd)) {
                return;  // we're done
              }
            }
          }
        }
      }
    }
  });
}

bool render_server::process(int fd) {
  while (true) {
    const std::string &buf = buffers[fd].get();
    const char *raw = buf.c_str();
    const size_t buf_len = buffers[fd].length();

    if (buf_len >= sizeof(int) * 2) {
      int *tmp = (int *)raw;
      int type = *tmp;
      int len = *(tmp + 1);
      if (len + (sizeof(int) * 2) <= buf_len) {
        if (!msg_callback(fd, type, size_t(len), buf.substr(sizeof(int) * 2, len))) {
          // terminate
          return false;
        }
        buffers[fd].erase_front((sizeof(int) * 2) + len);
      } else {
        break;
      }
    } else {
      break;
    }
  }

  const std::lock_guard<std::mutex> lock(buffers_mut);
  bool stop = false;
  while (!stop) {
    for (auto &pair : send_buffers) {
      const auto fd = pair.first;
      auto &buffer = pair.second;

      if (buffer.length() > 0) {
        int n = send(fd, buffer.get().c_str(), buffer.length(), 0);
        if (n == -1) {
          throw std::runtime_error("send failure, todo: fix");
        }
        buffer.erase_front(n);
      } else {
        stop = true;
      }
    }
  }
  return true;
}

int render_server::send_msg(int fd, int type, const char *data, int len_data) {
  const std::lock_guard<std::mutex> lock(buffers_mut);
  // send header
  int msg[] = {type, len_data};
  int len = sizeof(msg);
  send_buffers[fd].append((char *)&msg, len);

  // send data
  if (len_data == 0) return 0;
  send_buffers[fd].append(data, len_data);
  return 0;
}

void render_server::poll_and_send() {
  const std::lock_guard<std::mutex> lock(buffers_mut);
  for (auto &pair : send_buffers) {
    const auto fd = pair.first;
    auto &buffer = pair.second;

    if (buffer.length() > 0) {
      int n = send(fd, buffer.get().c_str(), buffer.length(), 0);
      if (n == -1) {
        throw std::runtime_error("send failure, todo: fix");
      }
      buffer.erase_front(n);
    }
  }
}

void render_server::send_job(int fd, bool to_files, int64_t timestamp, const data::job job) {
  std::ostringstream os;
  {
    cereal::BinaryOutputArchive archive(os);
    archive(to_files);
    archive(timestamp);
    archive(job);
  }
  int len = os.str().length();

  this->send_msg(fd, starcry_msgs::pull_job_response, os.str().c_str(), len);
}
