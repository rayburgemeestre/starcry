#include "job_cache.h"

job_cache::job_cache() {}

bool job_cache::take(data::job &job) {
  const std::lock_guard<std::mutex> lock(mut);
  cache_shapes[job.job_number].clear();
  std::swap(job.shapes, cache_shapes.at(job.job_number));
  return true;
}

std::vector<data::shape> job_cache::retrieve(data::job &job) {
  const std::lock_guard<std::mutex> lock(mut);
  auto ret = cache_shapes[job.job_number];
  cache_shapes.erase(job.job_number);
  return ret;
}

bool job_cache::take(data::pixel_data2 &dat) {
  const std::lock_guard<std::mutex> lock(mut);
  cache_pixels[dat.job_number] = {};
  std::swap(dat.pixels, cache_pixels.at(dat.job_number));
  return true;
}

std::vector<uint32_t> job_cache::retrieve(data::pixel_data2 &dat) {
  const std::lock_guard<std::mutex> lock(mut);
  auto ret = cache_pixels[dat.job_number];
  cache_pixels.erase(dat.job_number);
  return ret;
}
