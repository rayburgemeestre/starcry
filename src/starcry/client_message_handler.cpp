#include "starcry/client_message_handler.h"
#include "starcry.h"

#include "network/messages.h"
#include "network/render_server.h"
//#include "rendering_engine_wrapper.h"
//#include "bitmap_wrapper.hpp"
//
#include "cereal/archives/binary.hpp"
#include "starcry/metrics.h"

#include "generator.h"

client_message_handler::client_message_handler(starcry &sc) : sc(sc) {}

bool client_message_handler::on_client_message(int sockfd, int type, size_t len, const std::string &data) {
  switch (type) {
    case starcry_msgs::register_me: {
      sc.renderserver->send_msg(sockfd,
                                starcry_msgs::register_me_response,
                                (const char *)&sc.num_queue_per_worker,
                                sizeof(sc.num_queue_per_worker));
      sc.metrics_->register_thread(1000 + sockfd, "remote");
      break;
    }
    case starcry_msgs::pull_job: {
      while (true) {
        if (!sc.jobs->has_items(0)) {
          sc.jobs->sleep_until_items_available(0);
          if (!sc.jobs->active) {
            sc.frames->check_terminate();
            return false;  // shutdown server
          }
        }
        auto job = std::dynamic_pointer_cast<job_message>(sc.jobs->pop(0));
        if (job) {  // can be nullptr if someone else took it faster than us
          std::ostringstream os;
          cereal::BinaryOutputArchive archive(os);
          // TODO: perhaps there should be a better way than this
          job->job->is_raw = job->raw;
          archive(*(job->job));
          const auto settings = sc.gen->settings();
          archive(settings);
          sc.renderserver->send_msg(sockfd, starcry_msgs::pull_job_response, os.str().c_str(), os.str().size());
          if (job->job->job_number == std::numeric_limits<uint32_t>::max()) {
            sc.metrics_->set_frame_mode();
            sc.metrics_->render_job(1000 + sockfd, job->job->frame_number, job->job->chunk);
          } else {
            sc.metrics_->render_job(1000 + sockfd, job->job->job_number, job->job->chunk);
          }
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
      if (job.job_number == std::numeric_limits<uint32_t>::max()) {
        sc.metrics_->complete_render_job(1000 + sockfd, job.frame_number, job.chunk);
      } else {
        sc.metrics_->complete_render_job(1000 + sockfd, job.job_number, job.chunk);
      }
      if (!dat.pixels.empty()) {
        auto frame = std::make_shared<render_msg>(nullptr,
                                                  instruction_type::get_image,
                                                  job.job_number,
                                                  job.frame_number,
                                                  job.chunk,
                                                  job.num_chunks,
                                                  job.offset_x,
                                                  job.offset_y,
                                                  job.last_frame,
                                                  job.width,
                                                  job.height,
                                                  dat.pixels);
        sc.frames->push(frame);
      } else if (!dat.pixels_raw.empty()) {
        auto frame = std::make_shared<render_msg>(nullptr,
                                                  instruction_type::get_image,
                                                  job.job_number,
                                                  job.frame_number,
                                                  job.chunk,
                                                  job.num_chunks,
                                                  job.offset_x,
                                                  job.offset_y,
                                                  job.last_frame,
                                                  job.width,
                                                  job.height,
                                                  dat.pixels_raw);
        sc.frames->push(frame);
      }
    }
  }
  return true;
}
