/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sys/socket.h>
#include <cstdio>
#include <stdexcept>

int sendall(int s, const char *buf, int *len)
{
  int total = 0;        // how many bytes we've sent
  int bytesleft = *len; // how many we have left to send
  int n;

  while(total < *len) {
    n = send(s, buf+total, bytesleft, 0);
    if (n == -1) { break; }
    total += n;
    bytesleft -= n;
  }

  *len = total; // return number actually sent here

  return n==-1?-1:0; // return -1 on failure, 0 on success
}

int recvtimeout(int s, char *buf, int len, int timeout)
{
  fd_set fds;
  int n;
  struct timeval tv;

  // set up the file descriptor set
  FD_ZERO(&fds);
  FD_SET(s, &fds);

  // set up the struct timeval for the timeout
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  // wait until timeout or data received
  n = select(s+1, &fds, nullptr, nullptr, &tv);
  if (n == 0) return -2; // timeout!
  if (n == -1) return -1; // error

  // data must be here, so do a normal recv()
  return recv(s, buf, len, 0);
}

int send_msg(int fd, int type, const char *data, int len_data) {
  // send header
  int msg[] = {type, len_data};
  int len = sizeof(msg);
  if (sendall(fd, (char *) &msg, &len) == -1) {
    perror("sendall");
    printf("We only sent %d bytes because of the error!\n", len);
    throw std::runtime_error("todo");
  }

  if (len_data == 0) return 0;

  // send data
  len = len_data;
  if (sendall(fd, data, &len) == -1) {
    perror("sendall");
    printf("We only sent %d bytes because of the error!\n", len);
    throw std::runtime_error("todo");
  }
  return 0;
}
