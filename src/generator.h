/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <cstddef>
#include <cstdint>

#include "scalesettings.h"
#include "scenesettings.h"

#include "data/job.hpp"
#include "data/settings.hpp"
#include "data/texture.hpp"
#include "data/toroidal.hpp"

#include "util/frame_stepper.hpp"
#include "util/generator_context.h"
#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"

#include "util/quadtree.h"

class step_calculator;
class metrics;

namespace util {
namespace generator {
extern int64_t counter;
}
}  // namespace util

class generator {
private:
  std::shared_ptr<metrics> metrics_;
  std::shared_ptr<data::job> job;
  uint32_t frame_number = 0;

  size_t max_frames = 0;
  int32_t canvas_w = 0;
  int32_t canvas_h = 0;
  double seed = 1;
  double tolerated_granularity = 1;
  bool minimize_steps_per_object = true;
  size_t use_fps = 25;
  std::unordered_map<std::string, data::gradient> gradients;
  std::unordered_map<std::string, data::texture> textures;
  std::unordered_map<std::string, data::toroidal> toroidals;
  std::unordered_map<size_t, std::map<int, size_t>> indexes;
  frame_stepper stepper;
  std::unordered_map<int64_t, v8::Local<v8::Object>> parents;
  std::unordered_map<int64_t, v8::Local<v8::Object>> prev_parents;
  int attempt = 0;
  double max_dist_found = std::numeric_limits<double>::max();
  double sample_include = 0.;
  double sample_exclude = 0.;
  double sample_include_current = 0.;
  double sample_exclude_current = 0.;
  double total_skipped_frames = 0.;

  std::map<std::string, quadtree> qts;
  std::unordered_map<int64_t, size_t> next_instance_mapping;
  data::settings settings_;

  int min_intermediates = 0.;
  int max_intermediates = 30.;
  bool fast_ff = false;

  scale_settings scalesettings;
  scene_settings scenesettings;

  struct cache {
    bool enabled = false;
    bool use = false;
    std::unordered_map<size_t, bool> job_cache;
    std::map<size_t, std::unordered_map<int64_t, size_t>> next_instance_mapping;
    std::map<size_t, int64_t> counter;
    std::map<size_t, scale_settings> scalesettings;
    std::map<size_t, scene_settings> scenesettings;
  } cache;

  std::string filename_;

  generator_context genctx;

public:
  struct time_settings {
    double time;
    double elapsed;
    double scene_time;
  };

  explicit generator(std::shared_ptr<metrics>& metrics);
  ~generator() = default;

  void reset_context();
  void init(const std::string& filename, std::optional<double> rand_seed, bool preview, bool caching);
  void init_context();
  void init_user_script();
  void init_job();
  void init_video_meta_info(std::optional<double> rand_seed, bool preview);
  void init_gradients();
  void init_textures();
  void init_toroidals();
  void init_object_instances();

  void set_scene(size_t scene);
  void fast_forward(int frame_of_interest);
  bool generate_frame();
  void revert_all_changes(v8_interact& i,
                          v8::Local<v8::Array>& instances,
                          v8::Local<v8::Array>& next_instances,
                          v8::Local<v8::Array>& intermediates);
  static void revert_position_updates(v8_interact& i,
                                      v8::Local<v8::Array>& instances,
                                      v8::Local<v8::Array>& next_instances,
                                      v8::Local<v8::Array>& intermediates);
  void call_next_frame_event(v8_interact& i, v8::Local<v8::Array>& next_instances);
  void create_next_instance_mapping(v8_interact& i, v8::Local<v8::Array>& next_instances);
  void update_object_positions(v8_interact& i,
                               v8::Local<v8::Array>& next_instances,
                               int max_step,
                               v8::Local<v8::Object>& video);
  void update_object_toroidal(v8_interact& i, v8::Local<v8::Object>& instance, double& x, double& y);
  void update_object_interactions(v8_interact& i,
                                  v8::Local<v8::Array>& next_instances,
                                  v8::Local<v8::Array>& intermediates,
                                  v8::Local<v8::Array>& previous_instances,
                                  v8::Local<v8::Object>& video);
  void handle_collisions(v8_interact& i,
                         v8::Local<v8::Object> instance,
                         size_t index,
                         v8::Local<v8::Array> next_instances);
  static void handle_collision(v8_interact& i, v8::Local<v8::Object> instance, v8::Local<v8::Object> instance2);
  void update_time(v8_interact& i, v8::Local<v8::Object>& instance);
  int update_steps(double dist);
  double get_max_travel_of_object(v8_interact& i,
                                  v8::Local<v8::Object>& previous_instance,
                                  v8::Local<v8::Object>& instance

  );
  void convert_objects_to_render_job(v8_interact& i,
                                     v8::Local<v8::Array> next_instances,
                                     step_calculator& sc,
                                     v8::Local<v8::Object> video);
  void convert_object_to_render_job(
      v8_interact& i, v8::Local<v8::Object> instance, size_t index, step_calculator& sc, v8::Local<v8::Object> video);

  std::shared_ptr<data::job> get_job() const;
  double fps() const {
    return use_fps;
  }
  int32_t width() const {
    return canvas_w;
  }
  int32_t height() const {
    return canvas_h;
  }
  double get_seed() const {
    return seed;
  }
  size_t get_max_frames() const {
    return max_frames;
  }
  data::settings settings() const {
    return settings_;
  }
  std::string filename() const {
    return filename_;
  }

  scale_settings& get_scale_settings() {
    return scalesettings;
  }

  inline time_settings get_time() const;

  void fix(v8_interact& i, v8::Local<v8::Array>& instances);

  v8::Local<v8::Object> spawn_object(v8::Local<v8::Object> spawner, v8::Local<v8::Object> obj);

private:
  bool _generate_frame();
  bool _forward_latest_cached_frame(int frame_of_interest);
  void _load_cache();
  void _save_cache();
  void _load_js_cache(v8_interact& i,
                      v8::Local<v8::Object> cache,
                      v8::Local<v8::Array>& instances,
                      v8::Local<v8::Array>& intermediates,
                      v8::Local<v8::Array>& next_instances);
  void _save_js_cache(v8_interact& i,
                      v8::Local<v8::Object> cache,
                      v8::Local<v8::Array>* instances,
                      v8::Local<v8::Array>* intermediates,
                      v8::Local<v8::Array>* next_instances);
};

void call_print_exception(const std::string& fn);
template <typename T>
void call_print_exception(const std::string& fn, T arg);
