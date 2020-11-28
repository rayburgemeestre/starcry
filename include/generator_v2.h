/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <string>

#include <cstddef>
#include <cstdint>

#include "data/job.hpp"

#include "util/frame_stepper.hpp"
#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"

#include "util/quadtree.h"

class step_calculator;

class generator_v2 {
private:
  std::shared_ptr<data::job> job;
  // TODO: seems unused??
  uint32_t frame_number;

  size_t max_frames = 0;
  int32_t canvas_w = 0;
  int32_t canvas_h = 0;
  double tolerated_granularity = 1;
  size_t use_fps = 25;
  std::unordered_map<std::string, data::gradient> gradients;
  std::unordered_map<size_t, std::map<int, size_t>> indexes;
  frame_stepper stepper;
  std::unordered_map<int64_t, v8::Local<v8::Object>> parents;
  int attempt = 0;

  std::map<std::string, quadtree> qts;

public:
  generator_v2();
  ~generator_v2() = default;

  void init(const std::string& filename);
  void init_context(const std::string& filename);
  void init_user_script(const std::string& filename);
  void init_job();
  void init_video_meta_info();
  void init_gradients();
  void init_object_instances();

  bool generate_frame();
  void revert_all_changes(v8_interact& i,
                          v8::Local<v8::Array>& instances,
                          v8::Local<v8::Array>& next_instances,
                          v8::Local<v8::Array>& intermediates);
  void update_object_positions(v8_interact& i, v8::Local<v8::Array>& next_instances, int max_step);
  void update_object_interactions(v8_interact& i,
                                  v8::Local<v8::Array>& next_instances,
                                  v8::Local<v8::Array>& intermediates);
  void handle_collisions(v8_interact& i,
                         v8::Local<v8::Object> instance,
                         size_t index,
                         v8::Local<v8::Array> next_instances);
  void handle_collision(
      v8_interact& i, size_t index, size_t index2, v8::Local<v8::Object> instance, v8::Local<v8::Object> instance2);
  void update_time(v8_interact& i, v8::Local<v8::Object>& instance);
  int update_steps(double dist);
  double get_max_travel_of_object(v8_interact& i,
                                  v8::Local<v8::Object>& previous_instance,
                                  v8::Local<v8::Object>& instance);
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
};

void call_print_exception(const std::string& fn);
template <typename T>
void call_print_exception(const std::string& fn, T arg);
