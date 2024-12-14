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
#include "util/simple_split.hpp"

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

redis_server::redis_server(const std::string& host, starcry& sc) : host(host), sc(sc) {
  char hostname[1024] = {0x00};
  gethostname(hostname, 1024);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(100000, 200000);
  auto random_num = dis(gen);
  my_id_ = fmt::format("{}|{}", hostname, random_num);
}

redis_server::~redis_server() {
  running = false;
  runner.join();
  job_waiter.join();
}

void redis_server::run() {
  job_timeout_runner = std::thread([this]() {
    while (running) {
      // check if jobs in outstanding_jobs2 are already in the queue for more than 1 second
      // if they are: redispatch them
      for (auto& [job_id, job] : outstanding_jobs2) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - job.first).count();
        if (elapsed > 1000) {
          // first update the timestamp
          std::swap(job.first, now);

          // redispatch
          if (!dispatch_job(job.second)) {
            logger(INFO) << "redis_server - requeue dispatch failed: " << job_id.first << ", " << job_id.second
                         << std::endl;
          } else {
            logger(INFO) << "redis_server - requeue dispatch OK: " << job_id.first << ", " << job_id.second
                         << std::endl;
          }
        }
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });
  runner = std::thread([this]() {
    set_thread_name("redis_server::thread");
    while (running) {
      try {
        redis = std::make_unique<Redis>(host);
        auto sub = redis->subscriber();
        sub.on_message([&](const std::string& channel, const std::string& msg_in) -> void {
          logger(DEBUG) << "redis_server - msg on channel " << channel << std::endl;
          if (channel == "REGISTER") {
            logger(DEBUG) << "redis_server - REGISTER CLIENT: " << msg_in << std::endl;
            // split the string in msg which is HOSTNAME_ID into two variables
            auto [hostname, id, server_id] = split<3>(msg_in, '|');
            if (server_id.empty()) {
              server_id = my_id_;
            }
            const auto msg = fmt::format("{}|{}|{}", hostname, id, server_id);
            const auto worker_id = fmt::format("{}|{}", hostname, id);

            logger(DEBUG) << "redis_server - HOSTNAME: " << hostname << std::endl;
            logger(DEBUG) << "redis_server - ID: " << id << std::endl;
            logger(DEBUG) << "redis_server - SERVER_ID: " << server_id << std::endl;
            if (server_id == my_id_) {
              logger(DEBUG) << "redis_server - confirming worker_id: " << worker_id << std::endl;
              known_workers.insert(worker_id);
              redis->publish(worker_id, fmt::format("REGISTERED {}|{}", sc.num_queue_per_worker, server_id));
              sc.metrics_->register_thread(std::stoi(id), msg);
            } else {
              logger(DEBUG) << "redis_server - declining: " << msg << std::endl;
              redis->publish(worker_id, "DECLINED");
            }
          } else if (channel == "PULL_JOB") {
            const auto msg = msg_in;
            logger(DEBUG) << "redis_server - CLIENT READY FOR A JOB : " << msg << std::endl;
            // check if msg is one of the known_workers
            if (known_workers.contains(msg)) {
              logger(DEBUG) << "redis_server - ADDING KNOWN CLIENT: " << msg << std::endl;
              std::unique_lock lock(waiting_workers_mut);
              waiting_workers.push(msg);
              lock.unlock();

              // in case we already have a job lying around, send it right away..
              dispatch_job_from_queue();
            }
          } else if (channel == "ACK_JOB") {
            const auto msg = msg_in;
            logger(DEBUG) << "redis_server - CLIENT ACK RECEIVED: " << msg << std::endl;
            const auto& [hostname, id, job_num, job_chunk] = split<4>(msg, '|');
            const auto worker_id = std::format("{}|{}", hostname, id);

            auto job_number = std::stoi(job_num);
            auto job_chunk_num = std::stoi(job_chunk);
            auto find = outstanding_jobs2.find(std::make_pair(job_number, job_chunk_num));
            if (find == outstanding_jobs2.end()) {
              std::cerr << "redis_server - If you see this error, fire up the debugger! (2)" << std::endl;
              std::exit(1);
            }
            // TODO: lock
            outstanding_jobs3[find->first] = find->second.second;
            outstanding_jobs2.erase(find->first);  // deletes it now

          } else if (channel == "FRAME") {
            const auto msg = msg_in;
            try {
              logger(DEBUG) << "redis_server - CLIENT FRAME RECEIVED: " << msg.size() << std::endl;
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
              std::shared_ptr<job_message> related_job;
              if (find == outstanding_jobs2.end()) {
                auto find2 = outstanding_jobs3.find(std::make_pair(job.frame_number, job.chunk));
                if (find2 == outstanding_jobs3.end()) {
                  std::cerr << "redis_server - If you see this error, fire up the debugger!" << std::endl;
                  std::exit(1);
                }
                related_job = find2->second;
                logger(INFO) << "redis_server - Found job in outstanding_jobs3." << std::endl;
              } else {
                related_job = find->second.second;
                logger(INFO) << "redis_server - Found job in outstanding_jobs2." << std::endl;
              }
              logger(DEBUG) << "redis_server - job number and chunk: " << job.job_number << " " << job.chunk << " "
                            << std::boolalpha << job.last_frame << std::endl;
              logger(DEBUG) << "redis_server - job number and chunk: " << related_job->job->job_number << " "
                            << related_job->job->chunk << " " << std::boolalpha << related_job->job->last_frame
                            << std::endl;
              // remove the job from the outstanding_jobs2 and outstanding_jobs3 using std::erase
              {
                std::scoped_lock lock(dispatch_job_mut_);
                auto index = std::make_pair(job.frame_number, job.chunk);
                if (outstanding_jobs2.contains(index)) {
                  outstanding_jobs2.erase(index);
                }
                if (outstanding_jobs3.contains(index)) {
                  outstanding_jobs3.erase(index);
                }
              }
              sc.metrics_->complete_render_job(0, job.job_number, job.chunk);

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
        sub.subscribe("ACK_JOB");
        sub.subscribe("FRAME");

        redis->publish("RECONNECT", "RECONNECT");

        while (true) {
          try {
            sub.consume();
          } catch (const Error& err) {
            std::cout << "redis_server - Error (1): " << err.what() << std::endl;
            std::exit(1);
          }
        }
      } catch (const Error& e) {
        std::cerr << "redis_server - Error (2): " << e.what() << std::endl;
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
      dispatch_job_from_queue();
    }
  });
}

void redis_server::dispatch_job_from_queue() {
  std::scoped_lock lock(dispatch_job_mut_);
  if (waiting_workers.empty()) {
    logger(DEBUG) << "redis_server - Nothing to dispatch (race condition?)..." << std::endl;
    return;
  }

  auto job = std::dynamic_pointer_cast<job_message>(sc.jobs->pop(0));
  if (!dispatch_job(job)) {
    logger(DEBUG) << "redis_server - Nothing to dispatch (race condition?)..." << std::endl;
    return;
  }
}

bool redis_server::dispatch_job(std::shared_ptr<job_message> job) {
  if (job) {
    logger(INFO) << "redis_server - Preparing dispatch of job #" << job->job->job_number << std::endl;
    if (waiting_workers.empty()) {
      logger(DEBUG) << "redis_server - Nothing to dispatch (race condition?) (2)..." << std::endl;
      return false;
    }
    // sc.metrics_->set_render_job_object_state(0, job->job->job_number, job->job->chunk, 0,
    // metrics::job_state::rendered); // do these work??
    //  can be nullptr if someone else took it faster than us
    //  Next line might crash (dunno), if that's the case, we need to rewrite this to some std::optional..
    auto worker = pop_worker();  // after the worker finishes it will re-register
    logger(INFO) << "redis_server - Dispatching job #" << job->job->job_number << " to worker: " << worker << std::endl;
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
      return false;
    }
    job->job->is_raw = f ? (f->raw_bitmap() || f->raw_image()) : v->raw_video();
    archive(*(job->job));
    const auto settings = sc.gen->settings();
    archive(settings);
    bool include_objects_json = (f->metadata_objects() || sc.get_viewpoint().labels);
    archive(include_objects_json);
    const auto selected_ids_transitive = sc.selected_ids_transitive(job);
    archive(selected_ids_transitive);

    outstanding_jobs2[std::make_pair(job->job->frame_number, job->job->chunk)] =
        std::make_pair(std::chrono::high_resolution_clock::now(), job);
    // sc.renderserver->send_msg(sockfd, starcry_msgs::pull_job_response, os.str().c_str(),
    // os.str().size());
    logger(INFO) << "redis_server - Publishing job #" << job->job->job_number << " for worker: " << worker
                 << " (in redis)" << std::endl;

    // publish and immediate wait for ack from worker
    // also put WAIT_ACK state
    // then if ACK_JOB is received, then we move it to rendering, otherwise dispatch it again
    redis->publish(worker, fmt::format("JOB {}", os.str()));

    outstanding_jobs++;
    auto [hostname, id] = split<2>(worker, '|');
    sc.metrics_->set_frame_mode();
    if (job->job->job_number == std::numeric_limits<uint32_t>::max()) {
      sc.metrics_->render_job(std::stoi(id), job->job->frame_number, job->job->chunk);
    } else {
      sc.metrics_->render_job(std::stoi(id), job->job->job_number, job->job->chunk);
    }
    if (job->job->last_frame) {
      logger(DEBUG) << "redis_server - last frame has been send." << std::endl;
      send_last = true;
    }
  }
  return true;
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