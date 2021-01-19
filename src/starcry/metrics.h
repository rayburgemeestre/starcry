/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class metrics {
public:
  enum thread_state {
    idle,
    busy,
  };
  std::string str(thread_state in) {
    switch (in) {
      case thread_state::idle:
        return "IDLE";
      case thread_state::busy:
        return "BUSY";
    }
  }
  struct thread_data {
    int number;
    std::string name;
    metrics::thread_state state;
    std::chrono::time_point<std::chrono::high_resolution_clock> idle_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> idle_end;
    int job_number;
    int chunk;
  };

  enum job_state {
    queued,
    rendering,
    rendered,
  };
  std::string str(job_state in) {
    switch (in) {
      case job_state::queued:
        return "QUEUED";
      case job_state::rendering:
        return "RENDERING";
      case job_state::rendered:
        return "RENDERED";
    }
  }

  struct object {
    int number;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_end;
    job_state state;
  };

  struct chunk {
    int chunk;
    int num_chunks;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_end;
    metrics::job_state state;
    std::vector<metrics::object> objects;
  };

  struct job {
    int number;
    int frame;
    int chunk;
    int num_chunks;
    std::chrono::time_point<std::chrono::high_resolution_clock> generate_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> generate_end;
    int thread;
    std::vector<metrics::chunk> chunks;
  };
  enum class modes {
    global_mode,
    frame_mode,
  };
  metrics::modes mode = modes::global_mode;
  std::atomic<bool> record_objects{false};

private:
  std::map<int, metrics::thread_data> threads_;
  std::map<int, metrics::job> jobs_;
  std::mutex mut;

  int max_keep_jobs = 20;
  int y = 0;

  bool notty = false;
  bool nostdin = false;

  size_t completed_frames = 0;
  size_t max_frames;
  double hack_last_render_time = 0.;

public:
  explicit metrics(bool notty);
  ~metrics();

  void register_thread(int number, std::string desc);
  void register_job(int number, int frame, int chunk, int num_chunks);
  void render_job(int thread_number, int job_number, int chunk);
  void resize_job(int jbo_number, int num_chunks);
  void resize_job_objects(int number, int job_number, int chunk, int num_objects);
  void set_render_job_object_state(int thread_num, int job_num, int chunk_num, int index, metrics::job_state state);
  void complete_render_job(int thread_number, int job_number, int chunk);
  void complete_job(int number);
  void display();

  void set_frame_mode();
  void set_total_frames(size_t frames);

  void log_callback(int level, const std::string& line);

private:
  //  bool has_terminal();
  //  bool thread_exists(int number);

  double time_diff(std::chrono::time_point<std::chrono::high_resolution_clock> begin,
                   std::chrono::time_point<std::chrono::high_resolution_clock> end);

  std::thread curses;
  std::vector<std::pair<int, std::string>> ffmpeg;

  void output(int y, int x, std::string in);
};
