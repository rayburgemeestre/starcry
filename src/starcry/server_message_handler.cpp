#include "starcry/server_message_handler.h"
#include "starcry.h"

#include "bitmap_wrapper.hpp"
#include "network/messages.h"
#include "network/render_client.h"
#include "rendering_engine_wrapper.h"

#include "cereal/archives/binary.hpp"

#include <sstream>
#include <unistd.h>  // getpid()

server_message_handler::server_message_handler(starcry &sc) : sc(sc) {}

bool server_message_handler::on_server_message(render_client &client,
                                               rendering_engine_wrapper &engine,
                                               bitmap_wrapper &bitmap,
                                               int sockfd,
                                               int type,
                                               size_t len,
                                               const std::string &data) {
  switch (type) {
    case starcry_msgs::register_me_response: {
      if (len == sizeof(sc.num_queue_per_worker)) {
        memcpy(&sc.num_queue_per_worker, data.c_str(), sizeof(sc.num_queue_per_worker));
      }
      client.pull_job(true, 0);
      break;
    }
    case starcry_msgs::pull_job_response: {
      std::istringstream is(data);
      cereal::BinaryInputArchive archive(is);
      data::job job;
      archive(job);
      auto &bmp = bitmap.get(job.width, job.height);
      size_t num_shapes = 0;
      for (const auto &shapez : job.shapes) {
        num_shapes += shapez.size();
      }

      std::cout << "render client " << getpid() << " rendering job " << job.job_number << " shapes=" << num_shapes
                << ", dimensions=" << job.width << "x" << job.height << std::endl;
      sc.render_job(engine, job, bmp);
      data::pixel_data2 dat;
      dat.pixels = sc.pixels_vec_to_pixel_data(bmp.pixels());
      client.send_frame(job, dat, true);
      client.pull_job(true, 0);
      break;
    }
  }
  return true;
}
