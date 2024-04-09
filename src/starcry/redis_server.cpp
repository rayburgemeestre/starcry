/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry/redis_server.h"

#include "cereal/archives/binary.hpp"
#include "data/frame_request.hpp"
#include "data/video_request.hpp"
#include "generator.h"
#include "starcry.h"
#include "util/logger.h"

#include <fmt/core.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-extensions"
#endif
#include <sw/redis++/redis++.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

using namespace sw::redis;

redis_server::redis_server(const std::string& host, starcry& sc) : host(host), sc(sc) {}

redis_server::~redis_server() {
  running = false;
  runner.join();
  job_waiter.join();
}

namespace {
std::tuple<std::string, std::string> split(auto& msg) {
  if (auto pos = msg.find('_')) {
    return std::make_tuple(msg.substr(0, pos), msg.substr(pos + 1));
  }
  return std::make_tuple(msg, std::string());
}
}  // namespace

void redis_server::run() {
  runner = std::thread([this]() {
    set_thread_name("redis_server::thread");
    while (running) {
      try {
        redis = std::make_unique<Redis>(host);
        auto sub = redis->subscriber();
        sub.on_message([&](const std::string& channel, const std::string& msg) -> void {
          logger(DEBUG) << "msg on channel " << channel << std::endl;
          if (channel == "REGISTER") {
            logger(DEBUG) << "REGISTER CLIENT: " << msg << std::endl;
            // split the string in msg which is HOSTNAME_ID into two variables
            auto [hostname, id] = split(msg);

            logger(DEBUG) << "HOSTNAME: " << hostname << std::endl;
            logger(DEBUG) << "ID: " << id << std::endl;

            redis->publish(msg, fmt::format("REGISTERED {}", sc.num_queue_per_worker));
            sc.metrics_->register_thread(std::stoi(id), msg);
          } else if (channel == "PULL_JOB") {
            logger(DEBUG) << "CLIENT READY FOR A JOB : " << msg << std::endl;
            std::unique_lock lock(waiting_workers_mut);
            waiting_workers.push(msg);
            lock.unlock();

            // in case we already have a job lying around, send it right away..
            dispatch_job();
          } else if (channel == "FRAME") {
            try {
              logger(DEBUG) << "CLIENT FRAME RECEIVED: " << msg.size() << std::endl;
              std::istringstream is(msg);
              cereal::BinaryInputArchive archive(is);
              data::job job;
              data::pixel_data dat;
              bool is_remote;
              std::string buffer;
              archive(job);
              archive(dat);
              archive(is_remote);
              archive(buffer);
              // if (job.compress) {
              //   compress_vector<uint32_t> cv;
              //   cv.decompress(&dat.pixels, job.width * job.height);
              // }
              auto find = outstanding_jobs2.find(std::make_pair(job.frame_number, job.chunk));
              if (find == outstanding_jobs2.end()) {
                std::cerr << "If you see this error, fire up the debugger!" << std::endl;
                std::exit(1);
              }
              auto related_job = find->second;
              logger(DEBUG) << "job number and chunk: " << job.job_number << " " << job.chunk << " " << std::boolalpha
                            << job.last_frame << std::endl;

              if (job.last_frame) recv_last = true;
              // TODO: use separate message for this
              //              if (job.job_number == std::numeric_limits<uint32_t>::max()) {
              //                sc.metrics_->complete_render_job(std::stoi(id), job.frame_number, job.chunk);
              //              } else {
              //                sc.metrics_->complete_render_job(std::stoi(id), job.job_number, job.chunk);
              //              }
              auto frame = std::make_shared<render_msg>(related_job);
              if (!dat.pixels.empty()) {
                frame->set_pixels(dat.pixels);
              }
              if (!dat.pixels_raw.empty()) {
                frame->set_raw(dat.pixels_raw);
              }
              frame->set_buffer(buffer);
              sc.frames->push(frame);
              outstanding_jobs--;
              if (outstanding_jobs == 0) {
                if (!sc.jobs->active) {
                  sc.frames->check_terminate();
                }
              }
            } catch (cereal::Exception& ex) {
              logger(ERROR) << "Cereal exception: " << ex.what() << std::endl;
            }
          }
        });
        sub.subscribe("REGISTER");
        sub.subscribe("PULL_JOB");
        sub.subscribe("FRAME");

        redis->publish("RECONNECT", "RECONNECT");

        while (true) {
          try {
            sub.consume();
          } catch (const Error& err) {
            std::cout << "Error (1): " << err.what() << std::endl;
            std::exit(1);
          }
        }
      } catch (const Error& e) {
        std::cerr << "Error (2): " << e.what() << std::endl;
        std::exit(1);
      }
    }
  });
  job_waiter = std::thread([this]() {
    set_thread_name("job_waiter::thread");
    while (running) {
      // wait till we have something
      if (!sc.jobs->has_items(0)) {
        sc.jobs->sleep_until_items_available(0);
      }

      // wait till we have a worker available
      while (!workers_available()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }

      // start dispatching when we do
      dispatch_job();
    }
  });
}

void redis_server::dispatch_job() {
  std::scoped_lock lock(dispatch_job_mut_);
  if (waiting_workers.empty()) {
    logger(DEBUG) << "Nothing to dispatch (race condition?)..." << std::endl;
    return;
  }

  auto job = std::dynamic_pointer_cast<job_message>(sc.jobs->pop(0));
  if (job) {
    // can be nullptr if someone else took it faster than us
    // Next line might crash (dunno), if that's the case, we need to rewrite this to some std::optional..
    auto worker = pop_worker();  // after the worker finishes it will re-register
    std::ostringstream os;
    cereal::BinaryOutputArchive archive(os);
    std::shared_ptr<data::frame_request> f = nullptr;
    std::shared_ptr<data::video_request> v = nullptr;
    if (const auto& instruction = std::dynamic_pointer_cast<video_instruction>(job->original_instruction)) {
      v = instruction->video_ptr();
    } else if (const auto& instruction = std::dynamic_pointer_cast<frame_instruction>(job->original_instruction)) {
      f = instruction->frame_ptr();
    }
    if (!f && !v) {
      logger(WARNING) << "No video or frame instruction provided. (2)" << std::endl;
      return;
    }
    job->job->is_raw = f ? (f->raw_bitmap() || f->raw_image()) : v->raw_video();
    archive(*(job->job));
    const auto settings = sc.gen->settings();
    archive(settings);
    bool include_objects_json = (f->metadata_objects() || sc.get_viewpoint().labels);
    archive(include_objects_json);
    const auto selected_ids_transitive = sc.selected_ids_transitive(job);
    archive(selected_ids_transitive);

    outstanding_jobs2[std::make_pair(job->job->frame_number, job->job->chunk)] = job;
    // sc.renderserver->send_msg(sockfd, starcry_msgs::pull_job_response, os.str().c_str(),
    // os.str().size());
    redis->publish(worker, fmt::format("JOB {}", os.str()));
    outstanding_jobs++;
    auto [hostname, id] = split(worker);
    if (job->job->job_number == std::numeric_limits<uint32_t>::max()) {
      sc.metrics_->set_frame_mode();
      sc.metrics_->render_job(std::stoi(id), job->job->frame_number, job->job->chunk);
    } else {
      sc.metrics_->render_job(std::stoi(id), job->job->job_number, job->job->chunk);
    }
    if (job->job->last_frame) {
      logger(DEBUG) << "last frame has been send." << std::endl;
      send_last = true;
    }
  }
}

bool redis_server::workers_available() {
  std::scoped_lock lock(waiting_workers_mut);
  return !waiting_workers.empty();
}

std::string redis_server::pop_worker() {
  std::scoped_lock lock(waiting_workers_mut);
  auto elem = waiting_workers.top();
  waiting_workers.pop();
  return elem;
}