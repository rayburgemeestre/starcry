/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <atomic>

#include "util/socketbuffer.h"

class standard_output_to_logger {
public:
  standard_output_to_logger(std::ostream& stream, std::string prefix)
      : running(true),
        t(std::bind(&standard_output_to_logger::runner, this)),
        old(stream.rdbuf(buffer.rdbuf())),
        prefix(prefix) {}

  ~standard_output_to_logger() {
    std::cout.rdbuf(old);
    running = false;
    if (t.joinable()) t.join();
  }

private:
  void runner() {
    while (running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      {
        std::unique_lock<std::mutex> lock(mut);
        const auto buf = buffer.str();
        buffer.str("");
        buffer.clear();
        mybuff.append(buf.c_str(), buf.length());
      }
      while (mybuff.has_line()) {
        logger(INFO) << prefix << ": " << mybuff.get_line() << std::endl;
      }
    }
  }

  std::mutex mut;
  std::atomic<bool> running;
  std::thread t;
  std::stringstream buffer;
  std::streambuf* old = nullptr;
  socketbuffer mybuff;
  std::string prefix;
};