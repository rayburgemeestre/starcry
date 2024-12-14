/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <optional>
#include <unordered_map>

// TODO: wrap object definitions map into class so we can avoid this.
#include "util/v8_wrapper.hpp"
// And restore this.
// class v8_wrapper;

struct generator_options;
struct generator_state;
struct generator_config;

class scale_settings;
class metrics;

namespace data {
struct job;
struct settings;
}  // namespace data

namespace util {
class random_generator;
}

namespace data_staging {
class attrs;
};

namespace interpreter {
class gradient_manager;
class texture_manager;
class toroidal_manager;
class generator;  // soon remove
class job_mapper;
class object_lookup;
class scenes;
class bridges;
class frame_sampler;
class object_definitions;
class spawner;

class initializer {
public:
  explicit initializer(gradient_manager& gm,
                       texture_manager& tm,
                       toroidal_manager& toroidalman,
                       std::shared_ptr<v8_wrapper> context,
                       std::shared_ptr<metrics> metrics,
                       util::random_generator& rand_gen,
                       data_staging::attrs& attrs,
                       data::settings& settings,
                       object_lookup& lookup,
                       scale_settings& scalesettings,
                       bridges& bridges,
                       frame_sampler& sampler,
                       object_definitions& definitions,
                       const generator_options& options,
                       generator_state& state,
                       generator_config& config);

  void initialize_all(std::shared_ptr<data::job> job,
                      const std::string& filename,
                      std::optional<double> rand_seed,
                      bool preview,
                      std::optional<int> width,
                      std::optional<int> height,
                      std::optional<double> scale,
                      scenes& scenes_,
                      spawner& spawner_);

  void init_context(const std::string& filename);
  void init_user_script(spawner& spawner_);
  void init_video_meta_info(std::optional<double> rand_seed,
                            bool preview,
                            std::optional<int> width,
                            std::optional<int> height,
                            std::optional<double> scale,
                            scenes& scenes_);
  void init_gradients();
  void init_textures();
  void init_toroidals();
  void init_object_definitions();
  std::string get_js_api();

private:
  void reset_context();
  std::string serialize(const std::string& enum_type);

  gradient_manager& gradient_manager_;
  texture_manager& texture_manager_;
  toroidal_manager& toroidal_manager_;
  std::shared_ptr<v8_wrapper> context_;
  std::shared_ptr<job_mapper> job_mapper_ = nullptr;
  std::shared_ptr<metrics> metrics_;
  util::random_generator& rand_gen_;
  data_staging::attrs& global_attrs_;
  data::settings& settings_;
  object_lookup& object_lookup_;
  scale_settings& scale_settings_;
  bridges& bridges_;
  frame_sampler& frame_sampler_;
  object_definitions& object_definitions_;
  const generator_options& generator_options_;
  generator_state& generator_state_;
  generator_config& generator_config_;

  std::string js_api_;
};
}  // namespace interpreter