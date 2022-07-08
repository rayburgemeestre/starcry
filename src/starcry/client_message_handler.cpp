#include "starcry/client_message_handler.h"
#include "starcry.h"

#include "cereal/archives/binary.hpp"
#include "data/frame_request.hpp"
#include "data/video_request.hpp"
#include "network/messages.h"
#include "network/render_server.h"
#include "starcry/metrics.h"

#include "generator.h"
#include "native_generator.h"

client_message_handler::client_message_handler(starcry &sc) : sc(sc) {}

bool client_message_handler::on_client_message(int sockfd, int type, size_t len, const std::string &data) {
  switch (type) {
    case starcry_msgs::register_me: {
      sc.renderserver->send_msg(sockfd,
                                starcry_msgs::register_me_response,
                                (const char *)&sc.num_queue_per_worker,
                                sizeof(sc.num_queue_per_worker));
      sc.metrics_->register_thread(1000 + sockfd, data);
      break;
    }
    case starcry_msgs::pull_job: {
      if (send_last) {
        // TODO: proper handling, send some termination signal to client.
        break;
      }
      while (true) {
        if (!sc.jobs->has_items(0)) {
          sc.jobs->sleep_until_items_available(0);
        }
        auto job = std::dynamic_pointer_cast<job_message>(sc.jobs->pop(0));
        if (job) {  // can be nullptr if someone else took it faster than us
          std::ostringstream os;
          cereal::BinaryOutputArchive archive(os);
          auto f = job->original_instruction->frame_ptr();
          auto v = job->original_instruction->video_ptr();
          job->job->is_raw = f ? (f->raw_bitmap() || f->raw_image()) : v->raw_video();
          archive(*(job->job));
          const auto settings = sc.gen->settings();
          archive(settings);
          outstanding_jobs2[std::make_pair(job->job->frame_number, job->job->chunk)] = job;
          sc.renderserver->send_msg(sockfd, starcry_msgs::pull_job_response, os.str().c_str(), os.str().size());
          outstanding_jobs++;
          if (job->job->job_number == std::numeric_limits<uint32_t>::max()) {
            sc.metrics_->set_frame_mode();
            sc.metrics_->render_job(1000 + sockfd, job->job->frame_number, job->job->chunk);
          } else {
            sc.metrics_->render_job(1000 + sockfd, job->job->job_number, job->job->chunk);
          }
          if (job->job->last_frame) send_last = true;
          break;
        }
      }
      break;
    }
    case starcry_msgs::send_frame: {
      std::istringstream is(data);
      cereal::BinaryInputArchive archive(is);
      data::job job;
      data::pixel_data2 dat;
      bool is_remote;
      archive(job);
      archive(dat);
      archive(is_remote);
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

      if (job.last_frame) recv_last = true;
      if (job.job_number == std::numeric_limits<uint32_t>::max()) {
        sc.metrics_->complete_render_job(1000 + sockfd, job.frame_number, job.chunk);
      } else {
        sc.metrics_->complete_render_job(1000 + sockfd, job.job_number, job.chunk);
      }
      auto frame = std::make_shared<render_msg>(related_job);
      if (!dat.pixels.empty()) {
        frame->set_pixels(dat.pixels);
      }
      if (!dat.pixels_raw.empty()) {
        frame->set_raw(dat.pixels_raw);
      }
      sc.frames->push(frame);
      outstanding_jobs--;
      if (outstanding_jobs == 0) {
        if (!sc.jobs->active) {
          sc.frames->check_terminate();
          // shutdown server
          return false;
        }
      }
    }
  }
  return true;
}
