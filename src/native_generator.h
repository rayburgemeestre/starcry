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
#include "data_staging/shape.hpp"

#include "core/fps_progress.hpp"

#include "util/frame_stepper.hpp"
#include "util/native_generator_context.h"
#include "util/v8_interact.hpp"
#include "util/v8_wrapper.hpp"

#include "util/quadtree.h"

class v8_wrapper;
class step_calculator;
class metrics;
class vector2d;

namespace util {
namespace generator {
extern int64_t counter;
}
}  // namespace util

class native_generator {
private:
  fps_progress fpsp;
  std::shared_ptr<v8_wrapper> context;
  std::shared_ptr<metrics> metrics_;
  std::shared_ptr<data::job> job;
  std::vector<std::vector<data_staging::shape_t>> scene_shapes;
  std::vector<std::vector<data_staging::shape_t>> scene_shapes_next;
  std::vector<std::vector<data_staging::shape_t>> scene_shapes_intermediate;
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
  std::unordered_map<int64_t, v8::Local<v8::Object>> parents_stack;
  int attempt = 0;
  double max_dist_found = std::numeric_limits<double>::max();
  double sample_include = 0.;
  double sample_exclude = 0.;
  double sample_include_current = 0.;
  double sample_exclude_current = 0.;
  double total_skipped_frames = 0.;

  std::map<std::string, quadtree> qts;
  std::map<std::string, quadtree> qts_gravity;
  // TODO: Can we do without copies, please?
  std::unordered_map<int64_t, data_staging::shape_t> next_instance_map;
  std::unordered_map<int64_t, data_staging::shape_t> intermediate_map;
  std::unordered_map<std::string, v8::Persistent<v8::Object>> object_definitions_map;
  data::settings settings_;

  int min_intermediates = 1.;
  int max_intermediates = 30.;
  bool fast_ff = false;

  scale_settings scalesettings;
  scene_settings scenesettings;
  std::unordered_map<int64_t, scene_settings> scenesettings_objs;

  std::string filename_;

  std::shared_ptr<native_generator_context> genctx;

public:
  struct time_settings {
    double time;
    double elapsed;
    double scene_time;
  };

  explicit native_generator(std::shared_ptr<metrics>& metrics, std::shared_ptr<v8_wrapper>& context);
  ~native_generator() = default;

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
  void init_object_definitions();

  void instantiate_additional_objects_from_new_scene(v8::Persistent<v8::Array>& scene_objects,
                                                     v8::Local<v8::Object>* parent_object = nullptr);
  void create_bookkeeping_for_script_objects(v8::Local<v8::Object> created_instance);
  void set_scene(size_t scene);
  void set_scene_sub_object(scene_settings& scenesettings, size_t scene);
  void fast_forward(int frame_of_interest);
  bool generate_frame();
  void revert_all_changes(v8_interact& i);
  static void revert_position_updates(v8_interact& i);
  void call_next_frame_event(v8_interact& i, v8::Local<v8::Array>& next_instances);
  void create_new_mappings(v8_interact& i);
  void update_object_positions(v8_interact& i, v8::Local<v8::Object>& video);
  void update_object_toroidal(v8_interact& i, v8::Local<v8::Object>& instance, double& x, double& y);
  void update_object_interactions(v8_interact& i,
                                  v8::Local<v8::Array>& next_instances,
                                  v8::Local<v8::Array>& intermediates,
                                  v8::Local<v8::Array>& previous_instances,
                                  v8::Local<v8::Object>& video);
  void handle_collisions(v8_interact& i, v8::Local<v8::Object> instance, v8::Local<v8::Array> next_instances);
  void handle_gravity(v8_interact& i, v8::Local<v8::Object> instance, v8::Local<v8::Array> next_instances);
  void handle_collision(v8_interact& i, v8::Local<v8::Object> instance, v8::Local<v8::Object> instance2);
  void handle_gravity(v8_interact& i,
                      v8::Local<v8::Object> instance,
                      v8::Local<v8::Object> instance2,
                      vector2d& acceleration);
  void update_time(v8_interact& i,
                   data_staging::shape_t& instance,
                   const std::string& instance_id,
                   scene_settings& scenesettings);
  int update_steps(double dist);
  double get_max_travel_of_object(v8_interact& i,
                                  v8::Local<v8::Array>& next_instances,
                                  v8::Local<v8::Object>& previous_instance,
                                  v8::Local<v8::Object>& instance

  );
  void convert_objects_to_render_job(v8_interact& i, step_calculator& sc, v8::Local<v8::Object> video);
  void convert_object_to_render_job(v8_interact& i,
                                    data_staging::shape_t& shape,
                                    step_calculator& sc,
                                    v8::Local<v8::Object> video);

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

  inline time_settings get_time(scene_settings& settings) const;

  std::shared_ptr<v8_wrapper> get_context() const;

  v8::Local<v8::Object> spawn_object_native(v8::Local<v8::Object> spawner, v8::Local<v8::Object> obj);

private:
  bool _generate_frame();

  std::map<int64_t, std::pair<double, double>> cached_xy;
  void fix_xy(v8_interact& i, v8::Local<v8::Object>& instance, int64_t uid, double& x, double& y);

  v8::Local<v8::Object> _instantiate_object_from_scene(
      v8_interact& i,
      v8::Local<v8::Object>& scene_object,  // object description from scene to be instantiated
      v8::Local<v8::Object>* parent_object  // it's optional parent
  );
  void _instantiate_object(v8_interact& i,
                           std::optional<v8::Local<v8::Object>> scene_obj,
                           v8::Local<v8::Object> object_prototype,
                           v8::Local<v8::Object> new_instance,
                           int64_t level,
                           const std::string& namespace_);
};