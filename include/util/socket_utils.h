/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

int sendall(int s, char *buf, int *len);

int recvtimeout(int s, char *buf, int len, int timeout);

int send_msg(int fd, int type, const char *data, int len_data);
