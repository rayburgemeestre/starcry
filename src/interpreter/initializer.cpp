/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "initializer.h"

#include <utility>

#include "data/texture_effect.hpp"
#include "data/zernike_type.hpp"
#include "generator.h"
#include "gradient_factory.h"
#include "job_mapper.h"
#include "scripting.h"
#include "texture_factory.h"
#include "texture_manager.h"
#include "toroidal_factory.h"
#include "toroidal_manager.h"
#include "util/v8_interact.hpp"
#include "v8pp/module.hpp"

namespace interpreter {

v8::Local<v8::Context> current_context_native(std::shared_ptr<v8_wrapper>& wrapper_context) {
  return wrapper_context->context->isolate()->GetCurrentContext();
}

initializer::initializer(gradient_manager& gm,
                         texture_manager& tm,
                         toroidal_manager& toroidalman,
                         std::shared_ptr<v8_wrapper> context,
                         std::shared_ptr<metrics> metrics,
                         util::random_generator& rand_gen,
                         data_staging::attrs& global_attrs,
                         data::settings& settings,
                         object_lookup& lookup,
                         scenes& scenes_,
                         scale_settings& scalesettings,
                         bridges& bridges,
                         frame_sampler& sampler,
                         object_definitions& definitions,
                         const generator_options& options,
                         generator_state& state,
                         generator_config& config)
    : gradient_manager_(gm),
      texture_manager_(tm),
      toroidal_manager_(toroidalman),
      context_(std::move(context)),
      job_mapper_(nullptr),
      metrics_(std::move(metrics)),
      rand_gen_(rand_gen),
      global_attrs_(global_attrs),
      settings_(settings),
      object_lookup_(lookup),
      scenes_(scenes_),
      scalesettings_(scalesettings),
      bridges_(bridges),
      sampler_(sampler),
      definitions_(definitions),
      options_(options),
      state_(state),
      config_(config) {}

void initializer::initialize_all(std::shared_ptr<data::job> job,
                                 const std::string& filename,
                                 std::optional<double> rand_seed,
                                 bool preview,
                                 std::optional<int> width,
                                 std::optional<int> height,
                                 std::optional<double> scale) {
  job_mapper_ = std::make_shared<job_mapper>(job);
  init_context(filename);
  init_user_script();
  init_video_meta_info(rand_seed, preview, width, height, scale);
  init_gradients();
  init_textures();
  init_toroidals();
  init_object_definitions();
}

void initializer::init_context(const std::string& filename) {
  context_->set_filename(filename);
  reset_context();
}

void initializer::reset_context() {
  context_->reset();

  context_->add_fun("output", &output_fun);
  context_->add_fun("rand", [&]() {
    return rand_fun(rand_gen_);
  });
  context_->add_fun("rand1", [&](v8::Local<v8::Number> index) {
    rand_gen_.set_index(index->ToNumber(v8::Isolate::GetCurrent()->GetCurrentContext()).ToLocalChecked()->Value());
    return rand_fun(rand_gen_);
  });
  context_->add_fun("attr", [&](v8::Local<v8::String> str) -> v8::Local<v8::Value> {
    return global_attrs_.get(v8_str(v8::Isolate::GetCurrent(), str));
  });
  context_->add_fun("set_attr", [&](v8::Local<v8::String> key, v8::Local<v8::String> value) -> v8::Local<v8::Value> {
    global_attrs_.set(v8_str(v8::Isolate::GetCurrent(), key), v8_str(v8::Isolate::GetCurrent(), value));
    return v8::Boolean::New(v8::Isolate::GetCurrent(), true);
  });
  context_->add_fun("set_attr3",
                    [&](v8::Local<v8::String> object_id,
                        v8::Local<v8::String> key,
                        v8::Local<v8::String> value) -> v8::Local<v8::Value> {
                      const auto str = v8_str(v8::Isolate::GetCurrent(), object_id);
                      const auto object_id_long = std::stoll(str);
                      bool ret = false;
                      if (object_lookup_.contains(object_id_long)) {
                        auto& object_ref = object_lookup_.at(object_id_long).get();
                        meta_callback(object_ref, [&](auto& cc) {
                          cc.attrs_ref().set(v8_str(v8::Isolate::GetCurrent(), key),
                                             v8_str(v8::Isolate::GetCurrent(), value));
                          ret = true;
                        });
                      }
                      return v8::Boolean::New(v8::Isolate::GetCurrent(), ret);
                    });
  context_->add_fun("get_object", [&](v8::Local<v8::Number> unique_id) -> v8::Local<v8::Object> {
    return object_lookup_.get_object(
        unique_id->ToNumber(v8::Isolate::GetCurrent()->GetCurrentContext()).ToLocalChecked()->Value());
  });
  context_->add_fun("angled_velocity", &angled_velocity);
  context_->add_fun("random_velocity", [&]() {
    return random_velocity(rand_gen_);
  });
  context_->add_fun("random_velocity1", [&](v8::Local<v8::Number> index) {
    rand_gen_.set_index(index->ToNumber(v8::Isolate::GetCurrent()->GetCurrentContext()).ToLocalChecked()->Value());
    return random_velocity(rand_gen_);
  });
  context_->add_fun("expf", &expf_fun);
  context_->add_fun("logn", &logn_fun);
  context_->add_fun("clamp", &math::clamp<double>);
  context_->add_fun("squared", &squared);
  context_->add_fun("squared_dist", &squared_dist);
  context_->add_fun("get_distance", &get_distance);
  context_->add_fun("get_angle", &get_angle);
  context_->add_fun("triangular_wave", &triangular_wave);
  context_->add_fun("blending_type_str", &data::blending_type::to_str);
  context_->add_fun("texture_3d_str", &data::texture_3d::to_str);
  context_->add_fun("frame_number", [&]() {
    return job_mapper_->get_frame_number();
  });
  context_->add_fun("exit", &my_exit);

  context_->add_include_fun();

  // add blending constants
  v8::HandleScope scope(context_->context->isolate());
  v8pp::module consts(context_->isolate);
  consts.const_("normal", data::blending_type::normal)
      .const_("lighten", data::blending_type::lighten)
      .const_("darken", data::blending_type::darken)
      .const_("multiply", data::blending_type::multiply)
      .const_("average", data::blending_type::average)
      .const_("add", data::blending_type::add)
      .const_("subtract", data::blending_type::subtract)
      .const_("difference", data::blending_type::difference)
      .const_("negation", data::blending_type::negation_)
      .const_("screen", data::blending_type::screen)
      .const_("exclusion", data::blending_type::exclusion)
      .const_("overlay", data::blending_type::overlay)
      .const_("softlight", data::blending_type::softlight)
      .const_("hardlight", data::blending_type::hardlight)
      .const_("colordodge", data::blending_type::colordodge)
      .const_("colorburn", data::blending_type::colorburn)
      .const_("lineardodge", data::blending_type::lineardodge)
      .const_("linearburn", data::blending_type::linearburn)
      .const_("linearlight", data::blending_type::linearlight)
      .const_("vividlight", data::blending_type::vividlight)
      .const_("pinlight", data::blending_type::pinlight)
      .const_("hardmix", data::blending_type::hardmix)
      .const_("reflect", data::blending_type::reflect)
      .const_("glow", data::blending_type::glow)
      .const_("phoenix", data::blending_type::phoenix)
      .const_("hue", data::blending_type::hue)
      .const_("saturation", data::blending_type::saturation)
      .const_("color", data::blending_type::color)
      .const_("luminosity", data::blending_type::luminosity);
  context_->context->module("blending_type", consts);
  v8pp::module consts_2(context_->isolate);
  consts_2.const_("raw", data::texture_3d::raw)
      .const_("radial_displacement", data::texture_3d::radial_displacement)
      .const_("radial_compression", data::texture_3d::radial_compression)
      .const_("radial_distortion", data::texture_3d::radial_distortion)
      .const_("radial_scaling", data::texture_3d::radial_scaling)
      .const_("spherical", data::texture_3d::spherical)
      .const_("noise_3d_simplex", data::texture_3d::noise_3d_simplex)
      .const_("noise_3d_coords", data::texture_3d::noise_3d_coords);
  context_->context->module("texture_3d", consts_2);
  v8pp::module consts_3(context_->isolate);
  consts_3.const_("version1", data::zernike_type::version1).const_("version2", data::zernike_type::version2);
  context_->context->module("zernike_type", consts_3);

  v8pp::module consts_4(context_->isolate);
  consts_4.const_("opacity", data::texture_effect::opacity).const_("color", data::texture_effect::color);
  context_->context->module("texture_effect", consts_4);
}

void initializer::init_user_script() {
  std::ifstream stream(config_.filename.c_str());
  if (!stream && config_.filename != "-") {
    throw std::runtime_error("could not locate file " + config_.filename);
  }
  std::istreambuf_iterator<char> begin(config_.filename == "-" ? std::cin : stream), end;
  // remove "_ =" part from script (which is a workaround to silence 'not in use' linting)
  if (*begin == '_') {
    while (*begin != '=') begin++;
    begin++;
  }
  const auto source = std::string("script = ") + std::string(begin, end);
  context_->run("cache = typeof cache == 'undefined' ? {} : cache;");
  context_->run("script = {\"video\": {}};");
  try {
    context_->run(source);
  } catch (std::runtime_error& ex) {
    logger(WARNING) << "Exception occurred loading user script:\n" << ex.what() << std::endl;
  }
  v8::HandleScope hs(context_->context->isolate());
  bridges_.init();
}

void initializer::init_video_meta_info(std::optional<double> rand_seed,
                                       bool preview,
                                       std::optional<int> width,
                                       std::optional<int> height,
                                       std::optional<double> scale) {
  // "run_array" is a bit of a misnomer, this only invokes the callback once
  context_->run_array(
      "script", [this, &preview, &rand_seed, &width, &height, &scale](v8::Isolate* isolate, v8::Local<v8::Value> val) {
        v8_interact i;
        auto video_value = v8_index_object(current_context_native(context_), val, "video");
        auto video = video_value->IsObject() ? video_value.As<v8::Object>() : v8::Object::New(isolate);
        if (preview) {
          v8::Local<v8::Object> preview_obj;
          if (v8_index_object(current_context_native(context_), val, "preview")->IsObject()) {
            preview_obj = v8_index_object(current_context_native(context_), val, "preview").As<v8::Object>();
          } else {
            preview_obj = v8::Object::New(isolate);
          }
          if (!i.has_field(preview_obj, "width")) {
            i.set_field(preview_obj, "width", v8::Number::New(isolate, 1920 / 2.));
          }
          if (!i.has_field(preview_obj, "height")) {
            i.set_field(preview_obj, "height", v8::Number::New(isolate, 1080 / 2.));
          }
          if (!i.has_field(preview_obj, "max_intermediates")) {
            i.set_field(preview_obj, "max_intermediates", v8::Number::New(isolate, 5));
          }
          i.recursively_copy_object(video, preview_obj);
        }

        auto& duration = scenes_.scenesettings.scenes_duration;
        auto& durations = scenes_.scenesettings.scene_durations;
        durations.clear();
        auto obj = val.As<v8::Object>();
        auto scenes = i.v8_array(obj, "scenes");
        scenes_.reset();
        definitions_.clear();
        // TODO: put this in a meaningful function?
        object_lookup_.reset();
        for (size_t I = 0; I < scenes->Length(); I++) {
          scenes_.add_scene();
          auto current_scene = i.get_index(scenes, I);
          if (!current_scene->IsObject()) continue;
          auto sceneobj = current_scene.As<v8::Object>();
          duration += i.double_number(sceneobj, "duration");
          durations.push_back(i.double_number(sceneobj, "duration"));
        }
        std::for_each(durations.begin(), durations.end(), [&duration](double& n) {
          n /= duration;
        });

        config_.fps = i.double_number(video, "fps", 25);
        state_.canvas_w = i.double_number(video, "width", 1920);
        state_.canvas_h = i.double_number(video, "height", 1080);
        if (width && height) {
          state_.canvas_w = *width;
          state_.canvas_h = *height;
        }

        double use_scale = i.double_number(video, "scale", 1.);
        if (scale) {
          use_scale = *scale;
        }

        if (options_.custom_scale) use_scale = options_.custom_scale;
        if (options_.custom_width) state_.canvas_w = options_.custom_width;
        if (options_.custom_height) state_.canvas_h = options_.custom_height;
        if (options_.custom_granularity) config_.tolerated_granularity = options_.custom_granularity;
        if (options_.custom_grain) settings_.grain_for_opacity = *options_.custom_grain;

        state_.seed = rand_seed ? *rand_seed : i.double_number(video, "rand_seed");
        config_.tolerated_granularity = i.double_number(video, "granularity", 1);
        config_.minimize_steps_per_object = i.boolean(video, "minimize_steps_per_object", false);
        if (i.has_field(video, "perlin_noise")) settings_.perlin_noise = i.boolean(video, "perlin_noise");
        if (i.has_field(video, "motion_blur")) settings_.motion_blur = i.boolean(video, "motion_blur");
        if (i.has_field(video, "grain_for_opacity"))
          settings_.grain_for_opacity = i.boolean(video, "grain_for_opacity");
        if (i.has_field(video, "extra_grain")) settings_.extra_grain = i.double_number(video, "extra_grain");
        if (i.has_field(video, "update_positions")) settings_.update_positions = i.boolean(video, "update_positions");
        if (i.has_field(video, "dithering")) settings_.dithering = i.boolean(video, "dithering");
        if (i.has_field(video, "gamma")) settings_.gamma = i.double_number(video, "gamma");
        if (i.has_field(video, "scale_ratio")) settings_.scale_ratio = i.boolean(video, "scale_ratio");
        if (i.has_field(video, "min_intermediates"))
          config_.min_intermediates = i.integer_number(video, "min_intermediates");
        if (i.has_field(video, "max_intermediates"))
          config_.max_intermediates = i.integer_number(video, "max_intermediates");
        if (i.has_field(video, "fast_ff")) config_.fast_ff = i.boolean(video, "fast_ff");
        if (i.has_field(video, "bg_color")) {
          auto bg = i.v8_obj(video, "bg_color");
          job_mapper_->map_background_color(i, bg);
        }
        if (i.has_field(video, "sample")) {
          auto sample = i.get(video, "sample").As<v8::Object>();
          sampler_.set_sample_include(i.double_number(sample, "include"), config_.fps);  // seconds
          sampler_.set_sample_exclude(i.double_number(sample, "exclude"), config_.fps);  // seconds
        }
        rand_gen_.set_seed(state_.seed);

        state_.max_frames = duration * config_.fps;
        metrics_->set_total_frames(state_.max_frames);

        job_mapper_->map_canvas(state_.canvas_w, state_.canvas_h);

        job_mapper_->map_scale(use_scale);
        scalesettings_.video_scale = use_scale;
        scalesettings_.video_scale_next = use_scale;
        scalesettings_.video_scale_intermediate = use_scale;
      });
}

void initializer::init_gradients() {
  gradient_manager_.clear();
  context_->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i;
    auto obj = val.As<v8::Object>();
    auto gradient_objects = i.v8_obj(obj, "gradients");
    if (gradient_objects->IsNullOrUndefined()) return;
    auto gradient_fields = gradient_objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < gradient_fields->Length(); k++) {
      auto gradient_id = i.get_index(gradient_fields, k);
      auto gradient_value = i.get(gradient_objects, gradient_id);
      auto id = v8_str(isolate, gradient_id.As<v8::String>());
      // This code is duplicated in client.cpp, because there we are not dealing with V8 Javascript
      // objects, but nlohmann json objects.
      data::gradient new_gradient;
      if (gradient_value->IsArray()) {
        new_gradient = gradient_factory::create_from_array(gradient_value.As<v8::Array>(), i);
      } else if (gradient_value->IsString()) {
        auto color_string_obj = gradient_value.As<v8::String>();
        auto color_string = v8_str(isolate, color_string_obj);
        new_gradient = gradient_factory::create_from_string(color_string);
      }
      gradient_manager_.add_gradient(id, new_gradient);
    }
  });
}

void initializer::init_textures() {
  texture_manager_.clear();
  context_->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i;
    auto obj = val.As<v8::Object>();
    if (!i.has_field(obj, "textures")) {
      i.set_field(obj, "textures", v8::Object::New(isolate));
    }
    auto texture_objects = i.v8_obj(obj, "textures");
    auto texture_fields = texture_objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < texture_fields->Length(); k++) {
      auto texture_id = i.get_index(texture_fields, k);
      auto texture_settings = i.get(texture_objects, texture_id).As<v8::Object>();
      auto id = v8_str(isolate, texture_id.As<v8::String>());
      auto new_texture = texture_factory::create_from_object(i, texture_settings);
      texture_manager_.add_texture(id, new_texture);
    }
  });
}

void initializer::init_toroidals() {
  // toroidal_manager_.clear();
  context_->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i;
    auto obj = val.As<v8::Object>();
    if (!i.has_field(obj, "toroidal")) return;
    auto toroidal_objects = i.v8_obj(obj, "toroidal");
    auto toroidal_fields = toroidal_objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < toroidal_fields->Length(); k++) {
      auto toroidal_id = i.get_index(toroidal_fields, k);
      auto toroidal_settings = i.get(toroidal_objects, toroidal_id).As<v8::Object>();
      auto id = v8_str(isolate, toroidal_id.As<v8::String>());
      auto new_toroidal = toroidal_factory::create_from_object(i, toroidal_settings);
      toroidal_manager_.add_toroidal(id, new_toroidal);
    }
  });
}

void initializer::init_object_definitions() {
  context_->run("var object_definitions = {}");
  context_->enter_object("object_definitions", [this](v8::Isolate* isolate, v8::Local<v8::Value> object_definitions) {
    // we keep stuff here to avoid overwrites, we might be able to get rid of this in the future
    // but it's a one-time overhead, so doesn't matter too much.
    auto defs_storage = object_definitions.As<v8::Object>();
    context_->enter_object("script", [&, this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
      v8_interact i;
      auto obj = val.As<v8::Object>();
      auto object_definitions = i.v8_obj(obj, "objects");
      if (object_definitions->IsNullOrUndefined()) return;
      auto object_definitions_fields = object_definitions->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
      for (size_t k = 0; k < object_definitions_fields->Length(); k++) {
        auto object_id = i.get_index(object_definitions_fields, k);
        auto object_id_str = i.str(object_definitions_fields, k);
        logger(DEBUG) << "Importing object " << object_id_str << std::endl;
        auto object_definition = i.get(object_definitions, object_id).As<v8::Object>();
        i.set_field(defs_storage, object_id, object_definition);
        auto obj_from_storage = i.get(defs_storage, object_id).As<v8::Object>();
        auto id = v8_str(isolate, object_id.As<v8::String>());
        definitions_.update(id, obj_from_storage);
      }
    });
  });
}

}  // namespace interpreter
