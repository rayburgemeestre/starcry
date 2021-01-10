#include "starcry/metrics.h"
#include <fmt/core.h>

#include <unistd.h>  // isatty
#include <iostream>
#include <sstream>

#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

WINDOW *mainwin;
volatile bool flag = true;

void cleanup_curses() {
  flag = false;
  delwin(mainwin);
  endwin();
  refresh();
  std::cout << "\nWelcome back..." << std::endl;
}

void my_handler(int s) {
  printf("Caught signal %d\n", s);
  cleanup_curses();
  exit(1);
}

metrics::metrics() : exec(std::chrono::milliseconds(100)) {
  std::cout << "Switching to separate screen..." << std::endl;
  if ((mainwin = initscr()) == NULL) {
    fprintf(stderr, "Error initialising ncurses.\n");
    exit(EXIT_FAILURE);
  }
  mvaddstr(0, 0, "Welcome to Starcry!");
  clrtobot();

  timeout(1000);
  refresh();

  noecho();

  curses = std::thread([&]() {
    noecho();
    while (flag) {
      int c = getch();
      if (c != -1) {
        mvaddstr(1, 0, fmt::format("Last key pressed: {}  ", c).c_str());
      }
    }
  });

  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
}

metrics::~metrics() {
  cleanup_curses();
  for (const auto &line : ffmpeg) {
    if (line.first <= 32) {
      std::cout << line.second;
    }
  }
  curses.join();
}

void metrics::register_thread(int number, std::string desc) {
  std::unique_lock<std::mutex> lock(mut);
  threads_[number] = metrics::thread_data{number,
                                          std::move(desc),
                                          metrics::thread_state::idle,
                                          std::chrono::high_resolution_clock::now(),
                                          std::chrono::high_resolution_clock::now()};
  exec.run([&]() {
    display();
  });
}

void metrics::register_job(int number, int frame, int chunk, int num_chunks) {
  std::unique_lock<std::mutex> lock(mut);
  jobs_[number] = metrics::job{number,
                               frame,
                               chunk,
                               num_chunks,
                               std::chrono::high_resolution_clock::now(),
                               std::chrono::high_resolution_clock::now(),
                               -1,
                               {}};
  jobs_[number].chunks.push_back(metrics::chunk{1,
                                                1,
                                                std::chrono::high_resolution_clock::now(),
                                                std::chrono::high_resolution_clock::now(),
                                                metrics::job_state::queued});
  exec.run([&]() {
    display();
  });
}
void metrics::complete_job(int number) {
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(number) != jobs_.end()) {
    jobs_[number].generate_end = std::chrono::high_resolution_clock::now();
    exec.run([&]() {
      display();
    });
  }
}

void metrics::render_job(int thread_number, int job_number, int chunk) {
  std::unique_lock<std::mutex> lock(mut);
  if (threads_.find(thread_number) != threads_.end()) {
    threads_[thread_number].idle_end = std::chrono::high_resolution_clock::now();
    threads_[thread_number].state = metrics::thread_state::busy;
    threads_[thread_number].job_number = job_number;
    threads_[thread_number].chunk = chunk;
  }
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) {
      chunk--;
    }
    jobs_[job_number].chunks[chunk].render_begin = std::chrono::high_resolution_clock::now();
    jobs_[job_number].chunks[chunk].state = metrics::job_state::rendering;
    exec.run([&]() {
      display();
    });
  }
}

void metrics::resize_job(int number, int num_chunks) {
  std::unique_lock<std::mutex> lock(mut);
  if (jobs_.find(number) != jobs_.end()) {
    for (int i = 0; i < num_chunks; i++) {
      if (i == 0) {
        jobs_[number].chunks[0].num_chunks = num_chunks;
        continue;
      }
      jobs_[number].chunks.push_back(metrics::chunk{i + 1,
                                                    num_chunks,
                                                    std::chrono::high_resolution_clock::now(),
                                                    std::chrono::high_resolution_clock::now(),
                                                    metrics::job_state::queued});
    }
    exec.run([&]() {
      display();
    });
  }
}

void metrics::resize_job_objects(int number, int job_number, int chunk, int num_objects) {
  if (!record_objects) return;
  std::unique_lock<std::mutex> lock(mut);
  // TODO: Possibly do not need thread number ????
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) chunk--;
    for (int i = 0; i < num_objects; i++) {
      jobs_[job_number].chunks[chunk].objects.push_back(metrics::object{i,
                                                                        std::chrono::high_resolution_clock::now(),
                                                                        std::chrono::high_resolution_clock::now(),
                                                                        metrics::job_state::queued});
    }
    exec.run([&]() {
      display();
    });
  }
}

void metrics::set_render_job_object_state(
    int thread_num, int job_number, int chunk, int index, metrics::job_state state) {
  if (!record_objects) return;
  std::unique_lock<std::mutex> lock(mut);
  // TODO: Possibly do not need thread number ????
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) chunk--;
    jobs_[job_number].chunks[chunk].objects[index].state = state;
    if (state == metrics::job_state::rendering) {
      jobs_[job_number].chunks[chunk].objects[index].render_begin = std::chrono::high_resolution_clock::now();
    } else {
      jobs_[job_number].chunks[chunk].objects[index].render_end = std::chrono::high_resolution_clock::now();
    }
  }
  exec.run([&]() {
    display();
  });
}

void metrics::complete_render_job(int thread_number, int job_number, int chunk) {
  std::unique_lock<std::mutex> lock(mut);
  if (threads_.find(thread_number) != threads_.end()) {
    threads_[thread_number].idle_begin = std::chrono::high_resolution_clock::now();
    threads_[thread_number].state = metrics::thread_state::idle;
  }
  if (jobs_.find(job_number) != jobs_.end()) {
    if (chunk > 0) {
      chunk--;
    }
    jobs_[job_number].chunks[chunk].render_end = std::chrono::high_resolution_clock::now();
    jobs_[job_number].chunks[chunk].state = metrics::job_state::rendered;
  }
  exec.run([&]() {
    display();
  });
}

bool metrics::has_terminal() {
  return isatty(fileno(stdin));
}

bool metrics::thread_exists(int number) {
  return threads_.find(number) != threads_.end();
}

void metrics::display() {
  // locking here in theory is needed, but can cause slow updates because of a lot of locking going on
  // we delay 10ms between calls to display() anyway, so hopefully that will not cause issues.
  //  std::unique_lock<std::mutex> lock(mut);
  y = 2;
  for (const auto &thread : threads_) {
    std::stringstream ss;
    ss << "Thread: " << thread.first << " " << str(thread.second.state);
    if (thread.second.state == thread_state::idle) {
      ss << ": Seconds Idle: " << time_diff(thread.second.idle_begin, std::chrono::high_resolution_clock::now());
    } else if (thread.second.state == thread_state::busy) {
      ss << ": Job: " << thread.second.job_number << " Chunk: " << thread.second.chunk << " Seconds Busy: ";
      ss << time_diff(thread.second.idle_end, std::chrono::high_resolution_clock::now());
    }
    mvaddstr(y++, 0, ss.str().c_str());
    clrtobot();
  }
  y++;

  if (jobs_.empty()) return;
  auto first_key = jobs_.begin()->first;
  auto last_key = jobs_.rbegin()->first;

  if (jobs_.size() > max_keep_jobs) {
    auto delete_until_key = last_key - max_keep_jobs;
    if (first_key < delete_until_key) {
      for (auto i = first_key; i < delete_until_key; i++) {
        jobs_.erase(i);
      }
    }
  }

  first_key = jobs_.begin()->first;

  for (size_t i = first_key; i <= last_key; i++) {
    const auto &job = jobs_[i];
    int queued = 0;
    int rendering = 0;
    int rendered = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_begin = job.chunks[0].render_begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> render_end = job.chunks[1].render_begin;
    for (const auto &chunk : job.chunks) {
      if (chunk.state == job_state::queued) queued++;
      if (chunk.state == job_state::rendering) rendering++;
      if (chunk.state == job_state::rendered) rendered++;
      render_begin = std::min(render_begin, chunk.render_begin);
      render_end = std::max(render_end, chunk.render_end);
    }
    auto s = job_state::queued;
    if (queued < job.chunks.size()) {
      s = job_state::rendering;
      if (rendered < job.chunks.size()) {
        s = job_state::rendered;
      }
    }
    std::stringstream ss;
    switch (s) {
      case job_state::queued:
        ss << "Job: " << job.number << " " << str(s) << " "
           << "Gen.Time: " << time_diff(job.generate_begin, job.generate_end) << " ";
        break;
      case job_state::rendering:
        ss << "Job: " << job.number << " " << str(s) << " "
           << "Gen.Time: " << time_diff(job.generate_begin, job.generate_end) << " "
           << "Render Time: " << time_diff(render_begin, render_end) << " ";
        break;
      case job_state::rendered:
        ss << "Job: " << job.number << " " << str(s) << " "
           << "Gen.Time: " << time_diff(job.generate_begin, job.generate_end) << " "
           << "Render Time: " << time_diff(render_begin, render_end) << " ";
        break;
    }
    if (job.chunks.size() > 1) {
      if (mode == modes::global_mode) {
        ss << "Chunks: ";
        for (const auto &chunk : job.chunks) {
          ss << "" << chunk.state;
        }
        mvaddstr(y++, 0, ss.str().c_str());
        clrtobot();
        ss.clear();
        ss.str("");
      } else if (mode == modes::frame_mode) {
        mvaddstr(y++, 0, ss.str().c_str());
        clrtobot();
        ss.clear();
        ss.str("");
        for (const auto &chunk : job.chunks) {
          ss << " Chunk: " << chunk.chunk << " of " << chunk.num_chunks << " " << str(chunk.state) << " ";
          switch (chunk.state) {
            case job_state::queued:
              break;
            case job_state::rendering:
              ss << " Rendering T: " << time_diff(chunk.render_begin, std::chrono::high_resolution_clock::now()) << " ";
              break;
            case job_state::rendered:
              ss << " Rendered T: " << time_diff(chunk.render_begin, chunk.render_end) << " ";
              break;
          }
          for (const auto &obj : chunk.objects) {
            ss << obj.state;
          }
          mvaddstr(y++, 0, ss.str().c_str());
          clrtobot();
          ss.clear();
          ss.str("");
        }
        mvaddstr(y++, 0, ss.str().c_str());
        clrtobot();
        ss.clear();
        ss.str("");
      }
    } else {
      mvaddstr(y++, 0, ss.str().c_str());
      clrtobot();
      ss.clear();
      ss.str("");
    }
  }
  y++;
  for (int i = 0; i < 5; i++) {
    int index = ffmpeg.size() - 1 - i;
    if (index >= 0) {
      auto str = ffmpeg[index].second;
      mvaddstr(y++, 0, str.c_str());
      clrtobot();
    }
  }
}

void metrics::set_frame_mode() {
  mode = metrics::modes::frame_mode;
  record_objects = true;
}

void metrics::log_callback(int level, const std::string &line) {
  ffmpeg.push_back(std::make_pair(level, line));
}

double metrics::time_diff(std::chrono::time_point<std::chrono::high_resolution_clock> begin,
                          std::chrono::time_point<std::chrono::high_resolution_clock> end) {
  std::chrono::duration<double, std::milli> diff = end - begin;
  return diff.count() / 1000.;
}
