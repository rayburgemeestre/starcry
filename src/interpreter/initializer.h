/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <optional>

class v8_wrapper;

namespace data {
struct job;
}

namespace interpreter {
class gradient_manager;
class texture_manager;
class toroidal_manager;
class generator;  // soon remove
class job_mapper;

class initializer {
public:
  explicit initializer(generator& gen,
                       gradient_manager& gm,
                       texture_manager& tm,
                       toroidal_manager& toroidalman,
                       std::shared_ptr<v8_wrapper> context);

  void initialize_all(std::shared_ptr<data::job> job,
                      const std::string& filename,
                      std::optional<double> rand_seed,
                      bool preview,
                      std::optional<int> width,
                      std::optional<int> height,
                      std::optional<double> scale);

  void init_context(const std::string& filename);
  void init_user_script();
  void init_video_meta_info(std::optional<double> rand_seed,
                            bool preview,
                            std::optional<int> width,
                            std::optional<int> height,
                            std::optional<double> scale);
  void init_gradients();
  void init_textures();
  void init_toroidals();
  void init_object_definitions();

private:
  void reset_context();

  generator& gen_;
  gradient_manager& gradient_manager_;
  texture_manager& texture_manager_;
  toroidal_manager& toroidal_manager_;
  std::shared_ptr<v8_wrapper> context_;
  std::shared_ptr<job_mapper> job_mapper_;
};
}  // namespace interpreter