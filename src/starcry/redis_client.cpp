/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "starcry/redis_client.h"

#include "bitmap_wrapper.hpp"
#include "cereal/archives/binary.hpp"
#include "data/job.hpp"
#include "data/pixels.hpp"
#include "data/settings.hpp"
#include "starcry.h"
#include "util/image_utils.h"
#include "util/logger.h"

#include <fmt/core.h>
#include <linux/prctl.h>
#include <sw/redis++/redis++.h>
#include <sys/prctl.h>
#include <random>

using namespace sw::redis;

redis_client::redis_client(const std::string &host, starcry &sc) : host(host), sc(sc) {
  char hostname[1024] = {0x00};
  gethostname(hostname, 1024);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(100000, 200000);
  auto random_num = dis(gen);
  my_id_ = fmt::format("{}_{}", hostname, random_num);
}

redis_client::~redis_client() {}

void redis_client::run(bitmap_wrapper &bitmap, rendering_engine &engine) {
  try {
    // Create a Redis object, which is movable but NOT copyable.
    auto redis = Redis(host);
    redis.publish("REGISTER", my_id_);

    auto sub = redis.subscriber();
    sub.on_message([&](const std::string &channel, const std::string &msg) {
      // split msg into two parts: msg_type and data
      auto pos = msg.find(' ');
      auto msg_type = msg.substr(0, pos);
      auto payload = msg.substr(pos + 1);

      if (msg_type == "RECONNECT") {
        redis.publish("REGISTER", my_id_);
      } else if (msg_type == "REGISTERED") {
        logger(DEBUG) << "REGISTERED CLIENT RESPONSE DATA: " << payload << std::endl;
        // if (len == sizeof(sc.num_queue_per_worker)) {
        //   memcpy(&sc.num_queue_per_worker, data.c_str(), sizeof(sc.num_queue_per_worker));
        // }
        redis.publish("PULL_JOB", my_id_);
      } else if (msg_type == "JOB") {
        logger(DEBUG) << "GOT A JOB RESPONSE DATA: " << payload.size() << std::endl;

        std::istringstream is(payload);
        cereal::BinaryInputArchive archive(is);
        data::job job;
        data::settings settings;
        archive(job);
        archive(settings);
        auto &bmp = bitmap.get(job.width, job.height);
        size_t num_shapes = 0;
        for (const auto &shapez : job.shapes) {
          num_shapes += shapez.size();
        }

        std::cout << "render client " << getpid() << " rendering job " << job.job_number << " chunk = " << job.chunk
                  << " of " << job.num_chunks << ", shapes=" << num_shapes << ", dimensions=" << job.width << "x"
                  << job.height << std::endl;

        prctl(PR_SET_NAME,
              fmt::format("sc {} {}/{}", job.frame_number, job.chunk + 1, job.num_chunks).c_str(),
              NULL,
              NULL,
              NULL);

        sc.render_job(getpid(), engine, job, bmp, settings);
        data::pixel_data2 dat;
        if (job.is_raw) {
          dat.pixels_raw = bmp.pixels();
        } else {
          dat.pixels = pixels_vec_to_pixel_data(bmp.pixels(), settings.dithering);
        }

        std::ostringstream os;
        cereal::BinaryOutputArchive archive_out(os);
        //  double compression_ratio = 0;
        job.shapes.clear();
        //  if (job.compress) {
        //    compress_vector<uint32_t> cv;
        //    cv.compress(&dat.pixels, &compression_ratio);
        //  }
        archive_out(job);
        archive_out(dat);
        archive_out(true);
        redis.publish("FRAME", os.str());
        redis.publish("PULL_JOB", my_id_);
      }
    });
    sub.subscribe(my_id_);
    sub.subscribe("RECONNECT");

    while (true) {
      try {
        sub.consume();
      } catch (const Error &err) {
        std::cout << "Error: " << err.what() << std::endl;
      }
    }

  } catch (const Error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}