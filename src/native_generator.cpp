/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "native_generator.h"

#include <fmt/core.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <cmath>
#include <memory>
#include <mutex>
#include <numeric>

#include "v8pp/module.hpp"

#include "scripting.h"
#include "starcry/metrics.h"
#include "util/generator.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/random.hpp"
#include "util/step_calculator.hpp"
#include "util/vector_logic.hpp"

#include "data/coord.hpp"
#include "data/texture.hpp"
#include "data_staging/circle.hpp"
#include "shapes/circle.h"
#include "shapes/position.h"
#include "shapes/rectangle.h"

void meta_visit(auto& shape, auto&& handle_circle, auto&& handle_line, auto&& handle_text, auto&& handle_script) {
  std::visit(overloaded{[](std::monostate) {}, handle_circle, handle_line, handle_text, handle_script}, shape);
}

template <typename T>
void meta_callback(T& shape, auto&& callback) {
  meta_visit(
      shape,
      [&](data_staging::circle& c) {
        callback(c);
      },
      [&](data_staging::line& l) {
        callback(l);
      },
      [&](data_staging::text& t) {
        callback(t);
      },
      [&](data_staging::script& s) {
        callback(s);
      });
}

template <typename T>
void meta_callback(const T& shape, auto&& callback) {
  meta_visit(
      shape,
      [&](const data_staging::circle& c) {
        callback(c);
      },
      [&](const data_staging::line& l) {
        callback(l);
      },
      [&](const data_staging::text& t) {
        callback(t);
      },
      [&](const data_staging::script& s) {
        callback(s);
      });
}

void fix_properties(std::vector<std::vector<data_staging::shape_t>>& scene_shapes) {
  for (auto& shapes : scene_shapes) {
    for (auto& shape : shapes) {
      meta_callback(shape, [](auto& the_shape) {
        the_shape.properties_ref().reinitialize();
      });
    }
  }
}

native_generator::native_generator(std::shared_ptr<metrics>& metrics, std::shared_ptr<v8_wrapper>& context)
    : context(context), metrics_(metrics) {}

native_generator* global_native_generator = nullptr;

void native_generator::reset_context() {
  // reset context
  context->reset();

  context->add_fun("output", &output_fun);
  context->add_fun("rand", &rand_fun);
  context->add_fun("random_velocity", &random_velocity);
  context->add_fun("expf", &expf_fun);
  context->add_fun("logn", &logn_fun);
  context->add_fun("clamp", &math::clamp<double>);
  context->add_fun("squared", &squared);
  context->add_fun("squared_dist", &squared_dist);
  context->add_fun("get_distance", &get_distance);
  context->add_fun("get_angle", &get_angle);
  context->add_fun("triangular_wave", &triangular_wave);
  context->add_fun("blending_type_str", &data::blending_type::to_str);
  context->add_fun("exit", &my_exit);

  global_native_generator = this;
  context->add_include_fun();

  // add blending constants
  v8::HandleScope scope(context->context->isolate());
  v8pp::module consts(context->isolate);
  consts.set_const("normal", data::blending_type::normal)
      .set_const("lighten", data::blending_type::lighten)
      .set_const("darken", data::blending_type::darken)
      .set_const("multiply", data::blending_type::multiply)
      .set_const("average", data::blending_type::average)
      .set_const("add", data::blending_type::add)
      .set_const("subtract", data::blending_type::subtract)
      .set_const("difference", data::blending_type::difference)
      .set_const("negation", data::blending_type::negation_)
      .set_const("screen", data::blending_type::screen)
      .set_const("exclusion", data::blending_type::exclusion)
      .set_const("overlay", data::blending_type::overlay)
      .set_const("softlight", data::blending_type::softlight)
      .set_const("hardlight", data::blending_type::hardlight)
      .set_const("colordodge", data::blending_type::colordodge)
      .set_const("colorburn", data::blending_type::colorburn)
      .set_const("lineardodge", data::blending_type::lineardodge)
      .set_const("linearburn", data::blending_type::linearburn)
      .set_const("linearlight", data::blending_type::linearlight)
      .set_const("vividlight", data::blending_type::vividlight)
      .set_const("pinlight", data::blending_type::pinlight)
      .set_const("hardmix", data::blending_type::hardmix)
      .set_const("reflect", data::blending_type::reflect)
      .set_const("glow", data::blending_type::glow)
      .set_const("phoenix", data::blending_type::phoenix)
      .set_const("hue", data::blending_type::hue)
      .set_const("saturation", data::blending_type::saturation)
      .set_const("color", data::blending_type::color)
      .set_const("luminosity", data::blending_type::luminosity);
  context->context->set("blending_type", consts);
}
void native_generator::init(const std::string& filename, std::optional<double> rand_seed, bool preview, bool caching) {
  prctl(PR_SET_NAME, "native generator thread");
  filename_ = filename;
  init_context();
  init_user_script();
  init_job();
  init_video_meta_info(rand_seed, preview);
  init_gradients();
  init_textures();
  init_toroidals();

  context->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    genctx = std::make_shared<native_generator_context>(val, 0);
  });

  init_object_definitions();

  scenesettings.scene_initialized = std::numeric_limits<size_t>::max();

  // refresh the scene object to get rid of left-over state
  scene_settings tmp;
  std::swap(scenesettings, tmp);

  // restore scene durations info in the recreated object
  scenesettings.scene_durations = tmp.scene_durations;
  scenesettings.scenes_duration = tmp.scenes_duration;

  // throw away all the scene information for script objects
  scenesettings_objs.clear();

  // throw away any existing instances from array
  for (auto& scene_shapes : scene_shapes_next) scene_shapes.clear();

  // reset random number generator
  set_rand_seed(rand_seed.value_or(0));

  // set_scene requires generator_context to be set
  set_scene(0);

  // all objects added at this point can be blindly appended
  scene_shapes_next[scenesettings.current_scene_next].insert(
      std::end(scene_shapes_next[scenesettings.current_scene_next]),
      std::begin(instantiated_objects[scenesettings.current_scene_next]),
      std::end(instantiated_objects[scenesettings.current_scene_next]));
  instantiated_objects[scenesettings.current_scene_next].clear();
}

void native_generator::init_context() {
  context->set_filename(filename());
  reset_context();
}

void native_generator::init_user_script() {
  // }
  std::ifstream stream(filename().c_str());
  if (!stream && filename() != "-") {
    throw std::runtime_error("could not locate file " + filename());
  }
  std::istreambuf_iterator<char> begin(filename() == "-" ? std::cin : stream), end;
  // remove "_ =" part from script (which is a workaround to silence 'not in use' linting)
  if (*begin == '_') {
    while (*begin != '=') begin++;
    begin++;
  }
  const auto source = std::string("script = ") + std::string(begin, end);
  context->run("cache = typeof cache == 'undefined' ? {} : cache;");
  context->run("script = {\"video\": {}};");
  context->run(source);

  v8::HandleScope hs(context->context->isolate());
  object_bridge_circle = std::make_shared<object_bridge<data_staging::circle>>(this);
  object_bridge_line = std::make_shared<object_bridge<data_staging::line>>(this);
  object_bridge_text = std::make_shared<object_bridge<data_staging::text>>(this);
  object_bridge_script = std::make_shared<object_bridge<data_staging::script>>(this);
}

void native_generator::init_job() {
  job = std::make_shared<data::job>();
  job->background_color.r = 0;
  job->background_color.g = 0;
  job->background_color.b = 0;
  job->background_color.a = 0;
  job->width = 1920;   // canvas_w;
  job->height = 1080;  // canvas_h;
  job->job_number = 0;
  job->frame_number = frame_number;
  job->rendered = false;
  job->last_frame = false;
  job->num_chunks = 1;  // num_chunks;
  job->offset_x = 0;
  job->offset_y = 0;
  job->canvas_w = 1920;   // canvas_w;
  job->canvas_h = 1080;   // canvas_h;
  job->scale = 1.0;       // scale;
  job->compress = false;  // TODO: hardcoded
  job->save_image = false;
}

v8::Local<v8::Context> current_context_native(std::shared_ptr<v8_wrapper>& wrapper_context) {
  return wrapper_context->context->isolate()->GetCurrentContext();
}

void native_generator::init_video_meta_info(std::optional<double> rand_seed, bool preview) {
  // "run_array" is a bit of a misnomer, this only invokes the callback once
  context->run_array("script", [this, &preview, &rand_seed](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i;
    auto video = v8_index_object(current_context_native(context), val, "video").As<v8::Object>();
    if (preview) {
      v8::Local<v8::Object> preview_obj;
      if (v8_index_object(current_context_native(context), val, "preview")->IsObject()) {
        preview_obj = v8_index_object(current_context_native(context), val, "preview").As<v8::Object>();
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

    auto& duration = scenesettings.scenes_duration;
    auto& durations = scenesettings.scene_durations;
    durations.clear();
    auto obj = val.As<v8::Object>();
    auto scenes = i.v8_array(obj, "scenes");
    for (size_t I = 0; I < scenes->Length(); I++) {
      scene_shapes.emplace_back();
      scene_shapes_next.emplace_back();
      scene_shapes_intermediate.emplace_back();
      instantiated_objects.emplace_back();
      auto current_scene = i.get_index(scenes, I);
      if (!current_scene->IsObject()) continue;
      auto sceneobj = current_scene.As<v8::Object>();
      duration += i.double_number(sceneobj, "duration");
      durations.push_back(i.double_number(sceneobj, "duration"));
    }
    std::for_each(durations.begin(), durations.end(), [&duration](double& n) {
      n /= duration;
    });

    use_fps = i.double_number(video, "fps");
    canvas_w = i.double_number(video, "width");
    canvas_h = i.double_number(video, "height");
    seed = rand_seed ? *rand_seed : i.double_number(video, "rand_seed");
    tolerated_granularity = i.double_number(video, "granularity");
    minimize_steps_per_object = i.boolean(video, "minimize_steps_per_object");
    if (i.has_field(video, "perlin_noise")) settings_.perlin_noise = i.boolean(video, "perlin_noise");
    if (i.has_field(video, "motion_blur")) settings_.motion_blur = i.boolean(video, "motion_blur");
    if (i.has_field(video, "grain_for_opacity")) settings_.grain_for_opacity = i.boolean(video, "grain_for_opacity");
    if (i.has_field(video, "extra_grain")) settings_.extra_grain = i.double_number(video, "extra_grain");
    if (i.has_field(video, "update_positions")) settings_.update_positions = i.boolean(video, "update_positions");
    if (i.has_field(video, "dithering")) settings_.dithering = i.boolean(video, "dithering");
    if (i.has_field(video, "min_intermediates")) min_intermediates = i.integer_number(video, "min_intermediates");
    if (i.has_field(video, "max_intermediates")) max_intermediates = i.integer_number(video, "max_intermediates");
    if (i.has_field(video, "fast_ff")) fast_ff = i.boolean(video, "fast_ff");
    if (i.has_field(video, "bg_color")) {
      auto bg = i.v8_obj(video, "bg_color");
      job->background_color.r = i.double_number(bg, "r");
      job->background_color.g = i.double_number(bg, "g");
      job->background_color.b = i.double_number(bg, "b");
      job->background_color.a = i.double_number(bg, "a");
    }
    if (i.has_field(video, "sample")) {
      auto sample = i.get(video, "sample").As<v8::Object>();
      sample_include = i.double_number(sample, "include");  // seconds
      sample_exclude = i.double_number(sample, "exclude");  // seconds
      sample_include_current = sample_include * use_fps;
      sample_exclude_current = sample_exclude * use_fps;
    }
    set_rand_seed(seed);

    max_frames = duration * use_fps;
    metrics_->set_total_frames(max_frames);

    job->width = canvas_w;
    job->height = canvas_h;
    job->canvas_w = canvas_w;
    job->canvas_h = canvas_h;
    job->scale = i.double_number(video, "scale");

    scalesettings.video_scale = i.double_number(video, "scale");
    scalesettings.video_scale_next = i.double_number(video, "scale");
    scalesettings.video_scale_intermediate = i.double_number(video, "scale");
  });
}

void native_generator::init_gradients() {
  gradients.clear();
  context->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i;
    auto obj = val.As<v8::Object>();
    auto gradient_objects = i.v8_obj(obj, "gradients");
    auto gradient_fields = gradient_objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < gradient_fields->Length(); k++) {
      auto gradient_id = i.get_index(gradient_fields, k);
      auto positions = i.get(gradient_objects, gradient_id).As<v8::Array>();
      auto id = v8_str(isolate, gradient_id.As<v8::String>());
      for (size_t l = 0; l < positions->Length(); l++) {
        auto position = i.get_index(positions, l).As<v8::Object>();
        auto pos = i.double_number(position, "position");
        auto r = i.double_number(position, "r");
        auto g = i.double_number(position, "g");
        auto b = i.double_number(position, "b");
        auto a = i.double_number(position, "a");
        gradients[id].colors.emplace_back(pos, data::color{r, g, b, a});
      }
    }
  });
}

void native_generator::init_textures() {
  textures.clear();
  context->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i;
    auto obj = val.As<v8::Object>();
    if (!i.has_field(obj, "textures")) return;
    auto texture_objects = i.v8_obj(obj, "textures");
    auto texture_fields = texture_objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < texture_fields->Length(); k++) {
      auto texture_id = i.get_index(texture_fields, k);
      auto texture_settings = i.get(texture_objects, texture_id).As<v8::Object>();
      auto id = v8_str(isolate, texture_id.As<v8::String>());
      auto type = i.str(texture_settings, "type");
      textures[id].size = i.double_number(texture_settings, "size");
      textures[id].octaves = i.integer_number(texture_settings, "octaves");
      textures[id].persistence = i.double_number(texture_settings, "persistence");
      textures[id].percentage = i.double_number(texture_settings, "percentage");
      textures[id].scale = i.double_number(texture_settings, "scale");
      auto range = i.v8_array(texture_settings, "range");
      textures[id].strength = i.double_number(texture_settings, "strength");
      textures[id].speed = i.double_number(texture_settings, "speed");
      if (range->Length() == 4) {
        data::texture::noise_type use_type = data::texture::noise_type::perlin;
        if (type == "fractal") {
          use_type = data::texture::noise_type::fractal;
        } else if (type == "turbulence") {
          use_type = data::texture::noise_type::turbulence;
        }
        textures[id].type = use_type;
        textures[id].fromX = i.double_number(range, 0);
        textures[id].begin = i.double_number(range, 1);
        textures[id].end = i.double_number(range, 2);
        textures[id].toX = i.double_number(range, 3);
      }
    }
  });
}

void native_generator::init_toroidals() {
  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8_interact i;
    auto obj = val.As<v8::Object>();
    if (!i.has_field(obj, "toroidal")) return;
    auto toroidal_objects = i.v8_obj(obj, "toroidal");
    auto toroidal_fields = toroidal_objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
    for (size_t k = 0; k < toroidal_fields->Length(); k++) {
      auto toroidal_id = i.get_index(toroidal_fields, k);
      auto toroidal_settings = i.get(toroidal_objects, toroidal_id).As<v8::Object>();
      auto id = v8_str(isolate, toroidal_id.As<v8::String>());
      auto type = i.str(toroidal_settings, "type");
      toroidals[id].width = i.integer_number(toroidal_settings, "width");
      toroidals[id].height = i.integer_number(toroidal_settings, "height");
    }
  });
}

void native_generator::init_object_instances() {
  // This function is called whenever a scene is set. (once per scene)
  context->enter_object("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    // enter_objects creates a new isolate, using the old gives issues, so we'll recreate
    genctx = std::make_shared<native_generator_context>(val, scenesettings.current_scene_next);

    genctx->set_scene(scenesettings.current_scene_next);
    // auto& i = genctx->i();

    // whenever we switch to a new scene, we'll copy all the object state from the previous scene
    if (scenesettings.current_scene_next > 0) {
      logger(INFO) << "Switching to new scene, copying all state from previous." << std::endl;
      // auto prev_current_scene = i.get_index(genctx->scenes, scenesettings.current_scene_next - 1);
      // auto prev_sceneobj = prev_current_scene.As<v8::Object>();
      // continue from previous
      //      genctx->instances = i.v8_array(prev_sceneobj, "instances", v8::Array::New(isolate));
      //      genctx->instances_next = i.v8_array(prev_sceneobj, "instances_next", v8::Array::New(isolate));
      //      genctx->instances_intermediate = i.v8_array(prev_sceneobj, "instances_intermediate",
      //      v8::Array::New(isolate)); i.set_field(genctx->current_scene_obj, "instances", genctx->instances);
      //      i.set_field(genctx->current_scene_obj, "instances_next", genctx->instances_next);
      //      i.set_field(genctx->current_scene_obj, "instances_intermediate", genctx->instances_intermediate);
      scene_shapes[scenesettings.current_scene_next] = scene_shapes[scenesettings.current_scene_next - 1];
      scene_shapes_next[scenesettings.current_scene_next] = scene_shapes_next[scenesettings.current_scene_next - 1];
      scene_shapes_intermediate[scenesettings.current_scene_next] =
          scene_shapes_intermediate[scenesettings.current_scene_next - 1];
    }

    instantiate_additional_objects_from_new_scene(genctx->scene_objects);

    // since this is invoked directly after a scene change, and in the very beginning, make sure this state is part of
    // the instances "current" frame, or reverting (e.g., due to motion blur requirements) will discard all of this.
    ////util::generator::copy_instances(i, genctx->instances, genctx->instances_next);
    scene_shapes = scene_shapes_next;
    fix_properties(scene_shapes);
  });
}

void native_generator::init_object_definitions() {
  context->run("var object_definitions = {}");
  context->enter_object("object_definitions", [this](v8::Isolate* isolate, v8::Local<v8::Value> object_definitions) {
    // we keep stuff here to avoid overwrites, we might be able to get rid of this in the future
    // but it's a one-time overhead, so doesn't matter too much.
    auto defs_storage = object_definitions.As<v8::Object>();
    context->enter_object("script", [&, this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
      v8_interact i;
      auto obj = val.As<v8::Object>();
      auto object_definitions = i.v8_obj(obj, "objects");
      auto object_definitions_fields = object_definitions->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
      for (size_t k = 0; k < object_definitions_fields->Length(); k++) {
        auto object_id = i.get_index(object_definitions_fields, k);
        auto object_definition = i.get(object_definitions, object_id).As<v8::Object>();
        i.set_field(defs_storage, object_id, object_definition);
        auto obj_from_storage = i.get(defs_storage, object_id).As<v8::Object>();
        auto id = v8_str(isolate, object_id.As<v8::String>());
        object_definitions_map[id].Reset(isolate, obj_from_storage);
      }
    });
  });
}

void native_generator::create_bookkeeping_for_script_objects(v8::Local<v8::Object> created_instance,
                                                             const data_staging::shape_t& created_shape) {
  auto& i = genctx->i();

  // only do extra work for script objects
  if (i.str(created_instance, "type") != "script") {
    return;
  }

  // created shape namespace
  std::string created_shape_namespace;
  meta_callback(created_shape, [&]<typename T>(T& shape) {
    created_shape_namespace = shape.meta_cref().namespace_name();
  });

  const auto unique_id = i.integer_number(created_instance, "unique_id");
  const auto namespace_ = created_shape_namespace;
  i.set_field(created_instance, "namespace", v8_str(i.get_context(), namespace_));
  const auto file = i.str(created_instance, "file");
  const auto specified_duration =
      i.has_field(created_instance, "duration") ? i.double_number(created_instance, "duration") : double(-1);

  // read the entire script from disk
  std::ifstream stream(file);
  std::istreambuf_iterator<char> begin(stream), end;
  // remove "_ =" part from script (which is a workaround to silence 'not in use' linting)
  if (*begin == '_') {
    while (*begin != '=') begin++;
    begin++;
  }

  // evaluate script into temporary variable
  const auto source = std::string("var tmp = ") + std::string(begin, end) + std::string(";");
  context->run(source);
  auto tmp = i.get_global("tmp").As<v8::Object>();

  // process scenes and make the scenes relative, initialize helper objs etc
  auto scenes = i.v8_array(tmp, "scenes");
  auto& duration = scenesettings_objs[unique_id].scenes_duration;
  auto& durations = scenesettings_objs[unique_id].scene_durations;
  durations.clear();
  for (size_t I = 0; I < scenes->Length(); I++) {
    auto current_scene = i.get_index(scenes, I);
    if (!current_scene->IsObject()) continue;
    auto sceneobj = current_scene.As<v8::Object>();
    duration += i.double_number(sceneobj, "duration");
    durations.push_back(i.double_number(sceneobj, "duration"));
  }
  std::for_each(durations.begin(), durations.end(), [&duration](double& n) {
    n /= duration;
  });
  scenesettings_objs[unique_id].desired_duration = specified_duration;

  // make the scenes a property of the created instance (even though we probably won't need it for now)
  i.set_field(created_instance, "scenes", scenes);  // TODO: remove?

  // import all gradients from script
  auto gradients = i.v8_obj(tmp, "gradients");
  auto gradient_fields = gradients->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
  for (size_t k = 0; k < gradient_fields->Length(); k++) {
    auto gradient_src_id = i.get_index(gradient_fields, k);
    auto gradient_dst_id = namespace_ + i.str(gradient_fields, k);
    i.set_field(genctx->gradients, gradient_dst_id, i.get(gradients, gradient_src_id));
  }
  init_gradients();

  // import all object definitions from script
  auto objects = i.v8_obj(tmp, "objects");
  auto objects_fields = objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
  for (size_t k = 0; k < objects_fields->Length(); k++) {
    auto object_src_id = i.get_index(objects_fields, k);
    auto object_dst_id = namespace_ + i.str(objects_fields, k);
    // TODO: is this still needed?
    i.set_field(genctx->objects, object_dst_id, i.get(objects, object_src_id));
    auto val = i.get(objects, object_src_id);
    object_definitions_map[object_dst_id].Reset(i.get_isolate(), val.As<v8::Object>());
  }

  // make sure we start from the current 'global' time as an offset
  scenesettings_objs[unique_id].parent_offset = get_time(scenesettings).time;

  // sub object starts at scene zero
  set_scene_sub_object(scenesettings_objs[unique_id], 0);

  // recurse for each object in the "sub" scene
  auto current_scene = i.get_index(scenes, scenesettings_objs[unique_id].current_scene_next);
  if (current_scene->IsObject()) {
    auto o = current_scene.As<v8::Object>();
    auto scene_objects = i.v8_array(o, "objects");
    // TODO: why is it needed to convert these v8::Local objects? They seem to be garbage collected otherwise during
    // execution.
    v8::Persistent<v8::Array> tmp;
    tmp.Reset(i.get_isolate(), scene_objects);
    instantiate_additional_objects_from_new_scene(tmp, &created_shape);
  }

  // clean-up temporary variable that referenced the entire imported script
  context->run("tmp = undefined;");
}

void native_generator::instantiate_additional_objects_from_new_scene(v8::Persistent<v8::Array>& scene_objects,
                                                                     const data_staging::shape_t* parent_object) {
  auto& i = genctx->i();

  // instantiate all the additional objects from the new scene
  for (size_t j = 0; j < scene_objects.Get(i.get_isolate())->Length(); j++) {
    auto scene_obj = i.get_index(scene_objects, j).As<v8::Object>();

    auto [created_instance, shape_ref, shape_copy] = _instantiate_object_from_scene(i, scene_obj, parent_object);
    create_bookkeeping_for_script_objects(created_instance, shape_copy);
  }
}

void native_generator::set_scene(size_t scene) {
  if (scenesettings.current_scene_next == std::numeric_limits<size_t>::max())
    scenesettings.current_scene_next = scene;
  else
    scenesettings.current_scene_next = std::max(scenesettings.current_scene_next, scene);
  if (scenesettings.scene_initialized == std::numeric_limits<size_t>::max() ||
      scenesettings.current_scene_next > scenesettings.scene_initialized) {
    scenesettings.scene_initialized = scenesettings.current_scene_next;
    init_object_instances();
  }
}

void native_generator::set_scene_sub_object(scene_settings& scenesettings, size_t scene) {
  if (scenesettings.current_scene_next == std::numeric_limits<size_t>::max())
    scenesettings.current_scene_next = scene;
  else
    scenesettings.current_scene_next = std::max(scenesettings.current_scene_next, scene);
  if (scenesettings.scene_initialized == std::numeric_limits<size_t>::max() ||
      scenesettings.current_scene_next > scenesettings.scene_initialized) {
    scenesettings.scene_initialized = scenesettings.current_scene_next;
    // TODO: implement:
    //  init_object_instances();
  }
}

void native_generator::fast_forward(int frame_of_interest) {
  if (fast_ff && frame_of_interest > 2) {
    int backup_min_intermediates = min_intermediates;
    int backup_max_intermediates = max_intermediates;
    min_intermediates = 1;
    max_intermediates = 1;
    for (int i = 2; i < frame_of_interest; i++) {
      generate_frame();
      metrics_->skip_job(job->job_number);
    }
    min_intermediates = backup_min_intermediates;
    min_intermediates = backup_max_intermediates;
    // generate frame before with same stepsize
    generate_frame();
    metrics_->skip_job(job->job_number);
  } else if (frame_of_interest > 1) {
    for (int i = 1; i < frame_of_interest; i++) {
      generate_frame();
      metrics_->skip_job(job->job_number);
    }
  }
}

bool native_generator::generate_frame() {
  // no sampling
  if (sample_include == 0 || sample_exclude == 0) {
    return _generate_frame();
  }
  while (true) {
    // sampling frames to include
    if (sample_include_current-- > 0) {
      return _generate_frame();
    }
    // frames to be skipped
    while (sample_exclude_current-- > 0) {
      total_skipped_frames++;
      bool ret = _generate_frame();
      job->job_number--;
      if (!ret) {
        // bail out if we find a last frame
        return false;
      }
    }
    // reset
    sample_include_current = sample_include * use_fps;
    sample_exclude_current = sample_exclude * use_fps;
  }
}

bool native_generator::_generate_frame() {
  try {
    job->shapes.clear();

    // job_number is incremented later, hence we do a +1 on the next line.
    metrics_->register_job(job->job_number + 1, job->frame_number, job->chunk, job->num_chunks);

    context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
      genctx = std::make_shared<native_generator_context>(val, scenesettings.current_scene_next);
      auto& i = genctx->i();

      auto obj = val.As<v8::Object>();
      auto scenes = i.v8_array(obj, "scenes");
      auto video = i.v8_obj(obj, "video");
      // auto objects = i.v8_array(obj, "objects");
      auto current_scene = i.get_index(scenes, scenesettings.current_scene_next);
      if (!current_scene->IsObject()) return;
      // auto sceneobj = current_scene.As<v8::Object>();

      stepper.reset();
      indexes.clear();
      attempt = 0;
      max_dist_found = std::numeric_limits<double>::max();
      scalesettings.reset();

      // util::generator::garbage_collect_erased_objects(i, instances, intermediates, next_instances);

      if (min_intermediates > 0.) {
        update_steps(min_intermediates);
      }

      static const auto max_attempts = 2;
      while (max_dist_found > tolerated_granularity && !stepper.frozen) {
        if (++attempt >= max_attempts) {
          stepper.freeze();
        }
        logger(DEBUG) << "Generating frame [native] " << job->frame_number << " attempt " << attempt << std::endl;
        max_dist_found = 0;
        if (attempt > 1) {
          if (!settings_.motion_blur) break;
          revert_all_changes(i);
        }
        step_calculator sc(stepper.max_step);
        job->resize_for_num_steps(stepper.max_step);
        metrics_->set_steps(job->job_number + 1, attempt, stepper.max_step);

        stepper.rewind();
        bool detected_too_many_steps = false;
        while (stepper.has_next_step() && !detected_too_many_steps) {
          // logger(DEBUG) << "Stepper at step " << stepper.current_step << " out of " << stepper.max_step << std::endl;
          qts.clear();
          qts_gravity.clear();

          // initialize scene
          if (scenesettings.update(get_time(scenesettings).time)) {
            set_scene(scenesettings.current_scene_next + 1);
            // all objects added at this point can be blindly appended
            scene_shapes_next[scenesettings.current_scene_next].insert(
                std::end(scene_shapes_next[scenesettings.current_scene_next]),
                std::begin(instantiated_objects[scenesettings.current_scene_next]),
                std::end(instantiated_objects[scenesettings.current_scene_next]));
            instantiated_objects[scenesettings.current_scene_next].clear();
          }

          // initialize scenes for script objects
          for (auto& [_, settings] : scenesettings_objs) {
            if (settings.update(get_time(settings).time)) {
              set_scene_sub_object(settings, settings.current_scene_next + 1);
            }
          }

          // call next frame event on all objects (TODO: optimize)
          if (stepper.current_step == 0) {
            // call_next_frame_event(i, next_instances);
          }

          // create mappings
          create_new_mappings();

          // handle object movement (velocity added to position)
          update_object_positions(i, video);

          // handle collisions, gravity and "inherited" objects
          // absolute xy are known after this function call
          update_object_interactions(i, video);

          // calculate distance and steps
          update_object_distances();

          // above update functions could have triggered spawning of new objects
          insert_newly_created_objects();

          // convert javascript to renderable objects
          convert_objects_to_render_job(i, sc, video);

          scene_shapes_intermediate = scene_shapes_next;
          fix_properties(scene_shapes_intermediate);

          scalesettings.update();
          scenesettings.update();
          for (auto& iter : scenesettings_objs) {
            iter.second.update();
          }
          if (job->shapes.size() != size_t(stepper.max_step)) detected_too_many_steps = true;
          metrics_->update_steps(job->job_number + 1, attempt, stepper.current_step);
        }
        if (!detected_too_many_steps) {                 // didn't bail out with break above
          if (stepper.max_step == max_intermediates) {  // config doesn't allow finer granularity any way, break.
            break;
          } else if (stepper.max_step > max_intermediates) {
            logger(INFO) << "stepper.max_step > max_intermediates -> " << stepper.max_step << " > " << max_intermediates
                         << std::endl;
            std::exit(0);
            throw std::logic_error(
                fmt::format("stepper.max_step > max_intermediates ({} > {})", stepper.max_step, max_intermediates));
          }
        }
      }

      if (!settings_.update_positions) {
        revert_position_updates(i);
      }

      scene_shapes = scene_shapes_next;
      fix_properties(scene_shapes);

      scalesettings.commit();
      scenesettings.commit();
      fpsp.inc();
      for (auto& [_, scenesetting] : scenesettings_objs) {
        scenesetting.commit();
      }

      metrics_->update_steps(job->job_number + 1, attempt, stepper.current_step);
    });
    job->job_number++;
    job->frame_number++;
    metrics_->complete_job(job->job_number);
  } catch (std::exception& ex) {
    std::cout << ex.what() << std::endl;
  }
  job->last_frame = job->frame_number == max_frames;
  return job->frame_number != max_frames;
}

void native_generator::revert_all_changes(v8_interact& i) {
  job->shapes.clear();
  indexes.clear();

  // reset next and intermediate instances
  scene_shapes_next = scene_shapes;
  fix_properties(scene_shapes_next);

  scene_shapes_intermediate = scene_shapes;
  fix_properties(scene_shapes_intermediate);

  scalesettings.revert();
  scenesettings.revert();
  for (auto& iter : scenesettings_objs) {
    iter.second.revert();
  }
}

void native_generator::revert_position_updates(v8_interact& i) {
  return;  // todo
  //  for (size_t j = 0; j < next_instances->Length(); j++) {
  //    auto src = i.get_index(instances, j).As<v8::Object>();
  //    auto dst = i.get_index(next_instances, j).As<v8::Object>();
  //    auto dst2 = i.get_index(intermediates, j).As<v8::Object>();
  //    i.copy_field(dst, "x", src, "x");
  //    i.copy_field(dst, "y", src, "y");
  //    i.copy_field(dst, "x2", src, "x2");
  //    i.copy_field(dst, "y2", src, "y2");
  //    i.copy_field(dst2, "x", src, "x");
  //    i.copy_field(dst2, "y", src, "y");
  //    i.copy_field(dst2, "x2", src, "x2");
  //    i.copy_field(dst2, "y2", src, "y2");
  //  }
}

void native_generator::call_next_frame_event(v8_interact& i, v8::Local<v8::Array>& next_instances) {
  for (size_t j = 0; j < next_instances->Length(); j++) {
    auto next = i.get_index(next_instances, j).As<v8::Object>();
    auto on = i.get(next, "on").As<v8::Object>();
    i.call_fun(on, next, "next_frame");
  }
}

void native_generator::create_new_mappings() {
  next_instance_map.clear();
  intermediate_map.clear();
  for (auto& abstract_shape : scene_shapes_next[scenesettings.current_scene_next]) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      next_instance_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
  for (auto& abstract_shape : scene_shapes_intermediate[scenesettings.current_scene_next]) {
    // Will we just put copies here now??
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      intermediate_map.insert_or_assign(shape.meta_cref().unique_id(), std::ref(abstract_shape));
    });
  }
}

void native_generator::update_object_positions(v8_interact& i, v8::Local<v8::Object>& video) {
  // clear function caching
  cached_xy.clear();
  int64_t scenesettings_from_object_id = -1;
  int64_t scenesettings_from_object_id_level = -1;

  for (auto& abstract_shape : scene_shapes_next[scenesettings.current_scene_next]) {
    meta_callback(abstract_shape, [&]<typename T>(T& shape) {
      if constexpr (std::is_same_v<T, data_staging::script>) {
        // TODO: this strategy does not support nested script objects
        // TODO: we need to use stack for that
        scenesettings_from_object_id = shape.meta_cref().unique_id();
        scenesettings_from_object_id_level = shape.meta_cref().level();
      } else if (scenesettings_from_object_id_level == shape.meta_cref().level()) {
        scenesettings_from_object_id = -1;
        scenesettings_from_object_id_level = -1;
      }

      if (scenesettings_from_object_id == -1) {
        update_time(i, abstract_shape, shape.meta_cref().id(), scenesettings);
      } else {
        // TODO:
        update_time(i, abstract_shape, shape.meta_cref().id(), scenesettings_objs[scenesettings_from_object_id]);
      }

      scalesettings.video_scale_next = i.double_number(video, "scale");

      auto angle = shape.generic_cref().angle();
      if (std::isnan(angle)) {
        angle = 0.;
      }
      double x = 0;
      double y = 0;
      double x2 = 0;
      double y2 = 0;
      double velocity = 0;
      double vel_x = 0;
      double vel_y = 0;
      double vel_x2 = 0;
      double vel_y2 = 0;
      if constexpr (std::is_same_v<T, data_staging::line>) {
        x = shape.line_start_ref().position_cref().x;
        y = shape.line_start_ref().position_cref().y;
        x2 = shape.line_end_ref().position_cref().x;
        y2 = shape.line_end_ref().position_cref().y;
        velocity = shape.movement_line_start_ref().velocity_speed();
        vel_x = shape.movement_line_start_ref().velocity().x;
        vel_y = shape.movement_line_start_ref().velocity().y;
        vel_x2 = shape.movement_line_end_ref().velocity().x;
        vel_y2 = shape.movement_line_end_ref().velocity().y;
      } else {
        x = shape.location_cref().position_cref().x;
        y = shape.location_cref().position_cref().y;
        velocity = shape.movement_cref().velocity_speed();
        vel_x = shape.movement_cref().velocity().x;
        vel_y = shape.movement_cref().velocity().y;
      }

      velocity /= static_cast<double>(stepper.max_step);
      x += (vel_x * velocity);
      y += (vel_y * velocity);
      x2 += (vel_x2 * velocity);
      y2 += (vel_y2 * velocity);

      // For now we only care about circles
      // if (type == "circle" && i.double_number(instance, "radiussize") < 1000 /* todo create
      // property of course */) {
      if constexpr (std::is_same_v<T, data_staging::circle>) {
        if (shape.radius_size() < 1000 /* todo create property of course */) {
          // TODO:
          update_object_toroidal(i, shape.toroidal_ref(), x, y);
          const auto collision_group = shape.behavior_cref().collision_group();
          const auto gravity_group = shape.behavior_cref().gravity_group();
          if (!collision_group.empty()) {
            qts.try_emplace(collision_group,
                            quadtree(rectangle(position(-width() / 2, -height() / 2), width(), height()), 32));
            auto x_copy = x;
            auto y_copy = y;
            // TODO: fix
            //  fix_xy(i, instance, unique_id, x_copy, y_copy);
            qts[collision_group].insert(point_type(position(x_copy, y_copy), shape.meta_cref().unique_id()));
          }
          if (!gravity_group.empty()) {
            qts_gravity.try_emplace(gravity_group,
                                    quadtree(rectangle(position(-width() / 2, -height() / 2), width(), height()), 32));
            qts_gravity[gravity_group].insert(point_type(position(x, y), shape.meta_cref().unique_id()));
          }
        }
      }
      if constexpr (std::is_same_v<T, data_staging::line>) {
        shape.line_start_ref().position_ref().x = x;
        shape.line_start_ref().position_ref().y = y;
        shape.line_end_ref().position_ref().x = x2;
        shape.line_end_ref().position_ref().y = y2;
      } else {
        shape.location_ref().position_ref().x = x;
        shape.location_ref().position_ref().y = y;
      }
      // Needed?
      // if (attempt == 1) {
      //   i.set_field(instance, "steps", v8::Number::New(isolate, 1));
      // }
    });
  }
}

void native_generator::insert_newly_created_objects() {
  auto& dest = scene_shapes_next[scenesettings.current_scene_next];
  auto& source = instantiated_objects[scenesettings.current_scene_next];
  if (source.empty()) return;

  dest.reserve(dest.size() + source.size());

  auto handle = [&](auto& shape, const data_staging::meta& meta) {
    if (meta.parent_uid() == -1 /* no parent */) {
      dest.emplace_back(std::move(shape));
    } else {
      // calculate first where to insert instance in destination array
      const auto parent_uid = meta.parent_uid();
      size_t insert_offset = dest.size();
      int64_t found_level = 0;
      bool searching = false;
      for (size_t j = 0; j < dest.size(); j++) {
        auto& elem = dest[j];
        std::optional<std::reference_wrapper<const data_staging::meta>> elem_meta;
        meta_callback(elem, [&]<typename TP>(TP& shape) {
          elem_meta = shape.meta_cref();
        });
        const auto uid = elem_meta->get().unique_id();
        const auto level = elem_meta->get().level();
        if (searching && level <= found_level) {
          insert_offset = j;
          break;
        } else if (uid == parent_uid) {
          found_level = level;
          // assume at this point it's the element after this one
          insert_offset = j + 1;
          searching = true;
          // NOTE: we can early exit here to spawn new objects on top within their parent
          // We can make that feature configurable, or even add some z-index-like support
          // break;
        }
      }

      // reverse iterate over the destination array
      // note that we iterate from one beyond the size of the array, if size() = 3, [3] will potentially be out of
      // bounds
      for (size_t rev_index = dest.size(); rev_index; rev_index--) {
        // insert element where we calculated it should be
        if (rev_index == insert_offset) {
          if (insert_offset == dest.size()) {
            dest.emplace_back(std::move(shape));
          } else {
            dest[insert_offset] = std::move(shape);
          }
          // no need to process the rest of the array at this point
          break;
        }
        // for all elements move them down so that we create space for the new element
        if (rev_index > 0) {
          if (rev_index == dest.size()) {
            dest.emplace_back(std::move(dest[rev_index - 1]) /* element above */);
          } else {
            dest[rev_index] = std::move(dest[rev_index - 1]) /* element above */;
          }
        }
      }
    }
  };
  for (auto& abstract_shape : source) {
    meta_callback(abstract_shape, [&]<typename TP>(TP& shape) {
      handle(abstract_shape, shape.meta_cref());
    });
  }
  source.clear();
  create_new_mappings();
}

void native_generator::update_object_toroidal(v8_interact& i,
                                              data_staging::toroidal& toroidal_data,
                                              double& x,
                                              double& y) {
  if (toroidal_data.group().empty()) return;

  auto the_width = toroidals[toroidal_data.group()].width;
  auto the_height = toroidals[toroidal_data.group()].height;
  auto diff_x = 0;
  auto diff_y = 0;

  while (x + (the_width / 2) < 0) {
    x += the_width;
    diff_x += the_width;
  }
  while (y + (the_height / 2) < 0) {
    y += the_height;
    diff_y += the_height;
  }
  while (x + (the_width / 2) > the_width) {
    x -= the_width;
    diff_x -= the_width;
  }
  while (y + (the_height / 2) > the_height) {
    y -= the_height;
    diff_y -= the_height;
  }
  const auto warped_dist = get_distance(0, 0, diff_x, diff_y);
  toroidal_data.set_warp_width(the_width);
  toroidal_data.set_warp_height(the_height);
  toroidal_data.set_warp_dist(warped_dist);
}

void native_generator::update_object_interactions(v8_interact& i, v8::Local<v8::Object>& video) {
  const auto handle = [&](data_staging::shape_t& abstract_shape, data_staging::meta& meta) {
    if (meta.level() >= 0) {
      if (stack.size() <= meta.level()) {
        stack.emplace_back(abstract_shape);
      } else {
        stack[meta.level()] = std::ref(abstract_shape);
      }
    }
    handle_rotations(i, abstract_shape, scene_shapes_next[scenesettings.current_scene_next]);
    handle_collisions(i, abstract_shape, scene_shapes_next[scenesettings.current_scene_next]);
    handle_gravity(i, abstract_shape, scene_shapes_next[scenesettings.current_scene_next]);
  };
  stack.clear();
  for (auto& abstract_shape : scene_shapes_next[scenesettings.current_scene_next]) {
    std::visit(overloaded{[](std::monostate) {},
                          [&](data_staging::circle& c) {
                            handle(abstract_shape, c.meta_ref());
                            for (const auto& cascade_out : c.cascade_out_cref()) {
                              auto& other = next_instance_map.at(cascade_out.unique_id()).get();
                              if (auto other_line = std::get_if<data_staging::line>(&other)) {
                                if (cascade_out.type() == cascade_type::start) {
                                  other_line->line_start_ref().position_ref().x = c.location_ref().position_cref().x;
                                  other_line->line_start_ref().position_ref().y = c.location_ref().position_cref().y;
                                  other_line->transitive_line_start_ref().position_ref().x =
                                      c.transitive_location_ref().position_cref().x;
                                  other_line->transitive_line_start_ref().position_ref().y =
                                      c.transitive_location_ref().position_cref().y;
                                } else if (cascade_out.type() == cascade_type::end) {
                                  other_line->line_end_ref().position_ref().x = c.location_ref().position_cref().x;
                                  other_line->line_end_ref().position_ref().y = c.location_ref().position_cref().y;
                                  other_line->transitive_line_end_ref().position_ref().x =
                                      c.transitive_location_ref().position_cref().x;
                                  other_line->transitive_line_end_ref().position_ref().y =
                                      c.transitive_location_ref().position_cref().y;
                                }
                              }
                            }
                          },
                          [&](data_staging::line& l) {
                            handle(abstract_shape, l.meta_ref());
                          },
                          [&](data_staging::text& t) {
                            handle(abstract_shape, t.meta_ref());
                          },
                          [&](data_staging::script& s) {
                            handle(abstract_shape, s.meta_ref());
                          }},
               abstract_shape);
  }
}

void native_generator::update_object_distances() {
  stepper.reset_current();
  const auto handle = [&](data_staging::shape_t& abstract_shape, data_staging::meta& meta) {
    auto instance_uid = meta.unique_id();
    // MARK
    auto find = intermediate_map.find(instance_uid);
    if (find == intermediate_map.end()) {
      return;
    }
    double dist = get_max_travel_of_object(abstract_shape, find->second.get());
    if (dist > max_dist_found) {
      max_dist_found = dist;
    }
    auto steps = update_steps(dist);

    static std::unordered_map<int64_t, int> recorded_steps;

    if (attempt == 1) {
      meta.set_distance(dist);
      meta.set_steps(steps);
      recorded_steps[instance_uid] = steps;
    } else if (attempt > 1) {
      meta.set_steps(recorded_steps[instance_uid]);
    }
  };
  for (auto& abstract_shape : scene_shapes_next[scenesettings.current_scene_next]) {
    meta_callback(abstract_shape, [&]<typename TP>(TP& shape) {
      handle(abstract_shape, shape.meta_ref());
    });
  }
}

void native_generator::handle_rotations(v8_interact& i,
                                        data_staging::shape_t& shape,
                                        std::vector<data_staging::shape_t>& shapes) {
  data::coord pos;   // X, Y
  data::coord pos2;  // X2, Y2.
  data::coord parent;
  data::coord pos_for_parent;  // X, Y of circles or middle point of lines
  bool pivot_found = false;

  const auto handle = [&]<typename T>(data_staging::shape_t abstract_shape,
                                      T concrete_shape,
                                      data_staging::meta& meta,
                                      data_staging::generic& gen) {
    double angle = gen.angle();
    for (size_t i = 0; i <= meta.level(); i++) {
      auto& parent_shape = stack[i];
      meta_callback(parent_shape.get(), [&]<typename TP>(TP& parent_shape) {
        data::coord current;
        if constexpr (std::is_same_v<TP, data_staging::circle>) {
          current = {parent_shape.location_ref().position_ref().x, parent_shape.location_ref().position_ref().y};
        } else if constexpr (std::is_same_v<TP, data_staging::line>) {
          current = {parent_shape.line_start_ref().position_ref().x, parent_shape.line_start_ref().position_ref().y};
        } else if constexpr (std::is_same_v<TP, data_staging::script>) {
          current = {parent_shape.location_ref().position_ref().x, parent_shape.location_ref().position_ref().y};
        }

        // X2, Y2, and centerX, centerY, for lines
        data::coord current2;
        data::coord current_center;
        if constexpr (std::is_same_v<TP, data_staging::line>) {
          current2 =
              data::coord{parent_shape.line_end_ref().position_ref().x, parent_shape.line_end_ref().position_ref().y};
          // for lines current is middle of the line
          current_center.x = ((current.x - current2.x) / 2) + current2.x;
          current_center.y = ((current.y - current2.y) / 2) + current2.y;
        }

        // for now, let's go for the most intuitive choice
        pos.add(current);

        if constexpr (std::is_same_v<TP, data_staging::line>) {
          pos_for_parent.add(current_center);
          pos2.add(current2);
        } else {
          pos_for_parent.add(current);
        }

        if (parent_shape.meta_ref().is_pivot()) {
          pivot_found = true;
          parent = current;
        }

        // was this correct?
        double current_angle = parent_shape.generic_ref().angle();

        // total angle, cumulative no matter what
        // why would this shit be cumulative?
        if (!std::isnan(current_angle)) angle /*+*/ = current_angle;

        if (angle != 0.) {
          if constexpr (std::is_same_v<T, data_staging::line>) {
            // current angle + angle with parent
            auto angle1 = angle + get_angle(parent.x, parent.y, pos2.x, pos2.y);
            while (angle1 > 360.) angle1 -= 360.;
            auto rads = angle1 * M_PI / 180.0;
            auto ratio = 1.0;
            // rotates around its center
            auto dist = get_distance(current_center.x, current_center.y, current.x, current.y);
            auto move = dist * ratio * -1;
            if (false) {
              // auto dist = get_distance(current_center.x, current_center.y, current.x, current.y);
              // auto move = dist * ratio * -1;
              pos.x = parent.x + (cos(rads) * move) + current_center.x;
              pos.y = parent.y + (sin(rads) * move) + current_center.y;
              pos2.x = parent.x - (cos(rads) * move) + current_center.x;
              pos2.y = parent.y - (sin(rads) * move) + current_center.y;
            }
            // rotates in length
            if (true) {
              dist = get_distance(current.x, current.y, current2.x, current2.y);
              move = dist * ratio * -1;
              // pos.x = parent.x + (cos(rads) * move);
              // pos.y = parent.y + (sin(rads) * move);
              pos2.x = parent.x + (cos(rads) * move);
              pos2.y = parent.y + (sin(rads) * move);
            }
            // unsure
            pos_for_parent.x = parent.x + (cos(rads) * move);
            pos_for_parent.y = parent.y + (sin(rads) * move);
          } else {
            // current angle + angle with parent
            auto angle1 = angle + get_angle(parent.x, parent.y, pos.x, pos.y);
            while (angle1 > 360.) angle1 -= 360.;
            auto rads = angle1 * M_PI / 180.0;
            auto ratio = 1.0;
            // auto dist = get_distance(0, 0, current.x, current.y);
            auto dist = get_distance(pos.x, pos.y, parent.x, parent.y);
            auto move = dist * ratio * -1;
            pos.x = parent.x + (cos(rads) * move);
            pos.y = parent.y + (sin(rads) * move);
            pos_for_parent = pos;
          }
        }
        // now we can update the parent for the next level we're about to handle
        if (!pivot_found) parent = pos_for_parent;
      });
    }
  };
  meta_visit(
      shape,
      [&handle, &shape, &pos](data_staging::circle& c) {
        handle(shape, c, c.meta_ref(), c.generic_ref());
        c.transitive_location_ref().position_ref().x = pos.x;
        c.transitive_location_ref().position_ref().y = pos.y;
      },
      [&handle, &shape, &pos, &pos2](data_staging::line& l) {
        handle(shape, l, l.meta_ref(), l.generic_ref());
        bool skip_start = false, skip_end = false;
        for (const auto& cascade_in : l.cascades_in()) {
          if (cascade_in.type() == cascade_type::start) {
            skip_start = true;
          }
          if (cascade_in.type() == cascade_type::end) {
            skip_end = true;
          }
        }
        if (!skip_start) {
          l.transitive_line_start_ref().position_ref().x = pos.x;
          l.transitive_line_start_ref().position_ref().y = pos.y;
        }
        if (!skip_end) {
          l.transitive_line_end_ref().position_ref().x = pos2.x;
          l.transitive_line_end_ref().position_ref().y = pos2.y;
        }
      },
      [&handle, &shape, &pos](data_staging::text& t) {
        handle(shape, t, t.meta_ref(), t.generic_ref());
        t.transitive_location_ref().position_ref().x = pos.x;
        t.transitive_location_ref().position_ref().y = pos.y;
      },
      [&handle, &shape, &pos](data_staging::script& s) {
        handle(shape, s, s.meta_ref(), s.generic_ref());
        s.transitive_location_ref().position_ref().x = pos.x;
        s.transitive_location_ref().position_ref().y = pos.y;
      });
}

void native_generator::handle_collisions(v8_interact& i,
                                         data_staging::shape_t& shape,
                                         std::vector<data_staging::shape_t>& shapes) {
  // Now do the collision detection part
  // NOTE: we multiple radius/size * 2.0 since we're not looking up a point, and querying the quadtree does
  // not check for overlap, but only whether the x,y is inside the specified range. If we don't want to miss
  // points on the edge of our circle, we need to widen the matching range.
  // TODO: for different sizes of circle collision detection, we need to somehow adjust the interface to this
  // lookup somehow.
  std::vector<point_type> found;
  try {
    data_staging::circle& c = std::get<data_staging::circle>(shape);
    const auto& collision_group = c.behavior_ref().collision_group_ref();
    if (collision_group.empty() || collision_group == "undefined") {
      return;
    }
    auto x = c.location_ref().position_ref().x;
    auto y = c.location_ref().position_ref().y;
    auto unique_id = c.meta_cref().unique_id();
    // TODO: fix_xy(i, instance, unique_id, x, y);

    if (c.meta_cref().id() == "balls") return;

    auto radius = c.radius();
    auto radiussize = c.radius_size();

    qts[collision_group].query(unique_id, circle(position(x, y), radius * 2.0, radiussize * 2.0), found);
    if (radiussize < 1000 /* todo create property of course */) {
      for (const auto& collide : found) {
        const auto unique_id2 = collide.userdata;
        auto& shape2 = next_instance_map.at(unique_id2);
        try {
          data_staging::circle& c2 = std::get<data_staging::circle>(shape2.get());
          if (c2.meta_cref().id() != "balls" && c.meta_cref().unique_id() != c2.meta_cref().unique_id()) {
            handle_collision(i, c, c2, shape, shape2.get());
          }
        } catch (std::bad_variant_access const& ex) {
          // only supporting circles for now
          return;
        }
      }
    }
  } catch (std::bad_variant_access const& ex) {
    // only supporting circles for now
    return;
  }
}

void native_generator::handle_collision(v8_interact& i,
                                        data_staging::circle& instance,
                                        data_staging::circle& instance2,
                                        data_staging::shape_t& shape,
                                        data_staging::shape_t& shape2) {
  auto unique_id = instance.meta_cref().unique_id();
  auto unique_id2 = instance2.meta_cref().unique_id();
  auto last_collide = instance.behavior_ref().last_collide();

  auto x = instance.location_ref().position_cref().x;
  auto y = instance.location_ref().position_cref().y;
  // TODO: fix_xy(i, instance, unique_id, x, y);

  auto x2 = instance2.location_ref().position_cref().x;
  auto y2 = instance2.location_ref().position_cref().y;
  // TODO: fix_xy(i, instance2, unique_id2, x2, y2);

  auto radius = instance.radius();
  auto radiussize = instance.radius_size();
  auto radius2 = instance2.radius();
  auto radiussize2 = instance2.radius_size();
  auto mass = instance.generic_cref().mass();
  auto mass2 = instance2.generic_cref().mass();

  // If the quadtree reported a match, it doesn't mean the objects fully collide
  circle a(position(x, y), radius, radiussize);
  circle b(position(x2, y2), radius2, radiussize2);
  if (!a.overlaps(b)) return;

  // if not obj?
  if (instance.meta_cref().id() == "balls") return;  // experimental

  // they already collided, no need to let them collide again
  if (last_collide == unique_id2) return;

  // handle collision
  auto vel_x = instance.movement_ref().velocity().x;
  auto vel_y = instance.movement_ref().velocity().y;
  auto vel_x2 = instance2.movement_ref().velocity().x;
  auto vel_y2 = instance2.movement_ref().velocity().y;

  const auto normal = unit_vector(subtract_vector(vector2d(x, y), vector2d(x2, y2)));
  const auto ta = dot_product(vector2d(vel_x, vel_y), normal);
  const auto tb = dot_product(vector2d(vel_x2, vel_y2), normal);
  const auto optimized_p = (2.0 * (ta - tb)) / (mass + mass2);  // speed

  // save velocities
  const auto multiplied_vector = multiply_vector(normal, optimized_p);
  auto updated_vel1 = subtract_vector(vector2d(vel_x, vel_y), multiply_vector(multiplied_vector, mass2));
  instance.movement_ref().set_velocity(updated_vel1);
  auto updated_vel2 = add_vector(vector2d(vel_x2, vel_y2), multiply_vector(multiplied_vector, mass));
  instance2.movement_ref().set_velocity(updated_vel2);

  // save collision
  instance.behavior_ref().set_last_collide(unique_id2);
  instance2.behavior_ref().set_last_collide(unique_id);

  // collide callback
  auto find = object_definitions_map.find(instance.meta_cref().id());
  if (find != object_definitions_map.end()) {
    auto object_definition = v8::Local<v8::Object>::New(i.get_isolate(), find->second);
    auto handle_time_for_shape = [&](auto& c, auto& object_bridge_circle, auto other_unique_id) {
      object_bridge_circle->push_object(c);
      i.call_fun(object_definition, object_bridge_circle->instance(), "collide", other_unique_id);
      object_bridge_circle->pop_object();
    };
    meta_visit(
        shape,
        [&](data_staging::circle& c) {
          handle_time_for_shape(c, object_bridge_circle, unique_id2);
        },
        [&](data_staging::line& l) {
          handle_time_for_shape(l, object_bridge_line, unique_id2);
        },
        [&](data_staging::text& t) {
          handle_time_for_shape(t, object_bridge_text, unique_id2);
        },
        [&](data_staging::script& s) {
          handle_time_for_shape(s, object_bridge_script, unique_id2);
        });
    meta_visit(
        shape2,
        [&](data_staging::circle& c) {
          handle_time_for_shape(c, object_bridge_circle, unique_id);
        },
        [&](data_staging::line& l) {
          handle_time_for_shape(l, object_bridge_line, unique_id);
        },
        [&](data_staging::text& t) {
          handle_time_for_shape(t, object_bridge_text, unique_id);
        },
        [&](data_staging::script& s) {
          handle_time_for_shape(s, object_bridge_script, unique_id);
        });
  }
}

void native_generator::handle_gravity(v8_interact& i,
                                      data_staging::shape_t& shape,
                                      std::vector<data_staging::shape_t>& shapes) {
  try {
    std::vector<point_type> found;
    data_staging::circle& c = std::get<data_staging::circle>(shape);

    auto unique_id = c.meta_cref().unique_id();
    auto gravity_group = c.behavior_ref().gravity_group();
    if (gravity_group.empty()) {
      return;
    }

    if (c.movement_ref().velocity_speed() == 0) return;  // skip this one.

    auto x = c.location_ref().position_cref().x;
    auto y = c.location_ref().position_cref().y;
    // TODO: fix_xy(i, instance, unique_id, x, y);

    auto radius = c.radius();
    auto radiussize = c.radius_size();

    auto& video = genctx->video_obj;
    auto G = i.double_number(video, "gravity_G", 1);
    auto range = i.double_number(video, "gravity_range", 1000);
    const auto constrain_dist_min = i.double_number(video, "gravity_constrain_dist_min", 5.);
    const auto constrain_dist_max = i.double_number(video, "gravity_constrain_dist_max", 25.);

    qts_gravity[gravity_group].query(
        unique_id, circle(position(x, y), range + (radius * 2.0), range + (radiussize * 2.0)), found);

    if (c.radius_size() < 1000 /* todo create property of course */) {
      vector2d acceleration(0, 0);
      for (const auto& in_range : found) {
        const auto unique_id2 = in_range.userdata;
        auto shape2 = next_instance_map.at(unique_id2);
        try {
          data_staging::circle& c2 = std::get<data_staging::circle>(shape2.get());
          if (c.meta_cref().unique_id() != c2.meta_cref().unique_id()) {
            handle_gravity(i, c, c2, acceleration, G, range, constrain_dist_min, constrain_dist_max);
          }
        } catch (std::bad_variant_access const& ex) {
          // only supporting circles for now
          return;
        }
      }
      auto vel = add_vector(c.movement_ref().velocity(), acceleration);
      c.movement_ref().set_velocity(vel);
    }
  } catch (std::bad_variant_access const& ex) {
    // only supporting circles for now
    return;
  }
  //---
}

void native_generator::handle_gravity(v8_interact& i,
                                      data_staging::circle& instance,
                                      data_staging::circle& instance2,
                                      vector2d& acceleration,
                                      double G,
                                      double range,
                                      double constrain_dist_min,
                                      double constrain_dist_max) {
  // auto unique_id = instance.meta_cref().unique_id();
  auto x = instance.location_ref().position_cref().x;
  auto y = instance.location_ref().position_cref().y;
  // TODO: fix_xy(i, instance, unique_id, x, y);

  // auto unique_id2 = instance.meta_cref().unique_id();
  auto x2 = instance2.location_ref().position_cref().x;
  auto y2 = instance2.location_ref().position_cref().y;
  // TODO: fix_xy(i, instance2, unique_id2, x2, y2);

  auto radius = instance.radius();
  auto radiussize = instance.radius_size();
  auto radius2 = instance2.radius();
  auto radiussize2 = instance2.radius_size();
  auto mass = instance.generic_ref().mass();
  auto mass2 = instance2.generic_ref().mass();

  // If the quadtree reported a match, it doesn't mean the objects fully collide
  circle a(position(x, y), radius + range, radiussize);
  circle b(position(x2, y2), radius2 + range, radiussize2);
  double dist = 0;
  if (!a.overlaps(b, dist)) return;

  const auto constrained_distance = std::clamp(dist, constrain_dist_min, constrain_dist_max);

  vector2d vec_a(x, y);
  vector2d vec_b(x2, y2);
  const auto strength = (G * mass * mass2) / (constrained_distance * constrained_distance);
  auto force = subtract_vector(vec_b, vec_a);
  force = unit_vector(force);
  force = multiply_vector(force, strength / static_cast<double>(stepper.max_step));
  force = divide_vector(force, mass);

  acceleration.x += force.x;
  acceleration.y += force.y;
}

inline native_generator::time_settings native_generator::get_time(scene_settings& scenesettings) const {
  // Intermediate frames between 0 and 1, for two: [0.5, 1.0]
  // This will make vibrations look really vibrating, as back and forth will be rendered differently
  // auto extra = (static_cast<double>(stepper.next_step) / static_cast<double>(stepper.max_step));
  // Intermediate frames between 0 and 1, for two: [0.33, 0.66]
  // This will make vibrations invisible, as back and forth will be rendered the same way
  // NOTE: The only change is a +1 for max_step count.
  const auto extra = static_cast<double>(stepper.next_step) / static_cast<double>(stepper.max_step + 1);
  // Another fix, to make t= not start at for example -0.002, and run up to 0.995, is to make sure we start counting
  // at one, instead of zero, because we subtract something to account for intermediate frames
  // const auto fn = static_cast<double>(job->frame_number - (1.0 - extra));
  const auto fn = static_cast<double>(job->frame_number - (1.0 - extra) + 1);
  // Another fix, -1 on max_frames, so that we basically get 1 extra frame, it is often pleasing if the animation has
  // one final resting frame, f.i., to complete a full rotation. Otherwise you might see motion blur + rotation
  // stopped at 99.99%.
  // EDIT: NOTE, this -1 affects the calculation of properly "vibrating" objects with triangular_wave.
  // See for example the fix I had to make in input/wave.js
  // const auto t = std::clamp(fn / (max_frames - double(1.)), 0., 1.);
  // EDIT#2: reverted, to see if it fixes a bug of a possible endlessloop in the generate frame function
  const auto t = std::clamp(fn / (max_frames), 0., 1.);
  const auto e = static_cast<double>(1.0) / static_cast<double>(use_fps) / static_cast<double>(stepper.max_step);
  const auto next_scene_duration = scenesettings.scene_durations[scenesettings.current_scene_next];

  // block for special script type objects
  if (scenesettings.parent_offset != -1) {
    const auto desired_duration =
        scenesettings.desired_duration != -1 ? scenesettings.desired_duration : scenesettings.scenes_duration;
    const auto perc_of_t = desired_duration / this->scenesettings.scenes_duration;  // e.g. 0.33
    const auto Tstart = scenesettings.parent_offset;                                // e.g. 0.50
    const auto Tend = scenesettings.parent_offset + perc_of_t;                      // e.g. 0.83
    const auto T = (t - Tstart) / (Tend - Tstart);                                  // e.g. 0.91
    const auto E = e;
    auto S = std::clamp((T - scenesettings.offset_next) / next_scene_duration, 0., 1.);
    return time_settings{T, E, S};
  }

  // block for normal types
  const auto scene_time = std::clamp((t - scenesettings.offset_next) / next_scene_duration, 0., 1.);
  return time_settings{t, e, scene_time};
}

std::shared_ptr<v8_wrapper> native_generator::get_context() const {
  return context;
}

void native_generator::update_time(v8_interact& i,
                                   data_staging::shape_t& instance,
                                   const std::string& instance_id,
                                   scene_settings& scenesettings) {
  const auto time_settings = get_time(scenesettings);
  const auto execute = [&](double scene_time) {
    // Are these still needed?? (EDIT: I don't think it's used)
    // EDIT: during texture rendering we query the shape for the time
    // i.set_field(instance, "__time__", v8::Number::New(i.get_isolate(), scene_time));
    // i.set_field(instance, "__global_time__", v8::Number::New(i.get_isolate(), time_settings.time));
    // i.set_field(instance, "__elapsed__", v8::Number::New(i.get_isolate(), time_settings.elapsed));

    auto find = object_definitions_map.find(instance_id);
    if (find != object_definitions_map.end()) {
      auto object_definition = v8::Local<v8::Object>::New(i.get_isolate(), find->second);
      auto handle_time_for_shape = [&](auto& c, auto& object_bridge) {
        // TODO: check if the object has an "time" function, or we can just skip this entire thing
        c.meta_ref().set_time(scene_time);
        object_bridge->push_object(c);
        i.call_fun(object_definition,
                   object_bridge->instance(),
                   "time",
                   scene_time,
                   time_settings.elapsed,
                   scenesettings.current_scene_next,
                   time_settings.time);
        object_bridge->pop_object();
      };
      meta_visit(
          instance,
          [&](data_staging::circle& c) {
            handle_time_for_shape(c, object_bridge_circle);
          },
          [&](data_staging::line& l) {
            handle_time_for_shape(l, object_bridge_line);
          },
          [&](data_staging::text& t) {
            handle_time_for_shape(t, object_bridge_text);
          },
          [&](data_staging::script& s) {
            handle_time_for_shape(s, object_bridge_script);
          });
    }
  };

  if (scenesettings.current_scene_next > scenesettings.current_scene_intermediate) {
    // Make sure we end previous scene at the very last frame in any case, even though we won't render it.
    // This may be necessary to finalize some calculations that work with "t" (time), i.e., for rotations.
    auto bak = scenesettings.current_scene_next;
    scenesettings.current_scene_next = scenesettings.current_scene_intermediate;
    execute(1.0);
    scenesettings.current_scene_next = bak;
  }
  execute(time_settings.scene_time);
}

int native_generator::update_steps(double dist) {
  auto steps = round(std::max(1.0, fabs(dist) / tolerated_granularity));
  if (steps > max_intermediates) {
    steps = max_intermediates;
  }
  stepper.update(steps);
  return steps;
}

// TODO: there is too much logic in this function that is now clearly in the wrong place.
// This will be refactored soon.
double native_generator::get_max_travel_of_object(data_staging::shape_t& shape_now, data_staging::shape_t& shape_prev) {
  vector2d xy_now;
  vector2d xy_prev;
  vector2d xy2_now;
  vector2d xy2_prev;
  bool compare_xy2 = false;

  double radius_now = 0;
  double radius_prev = 0;

  meta_visit(
      shape_now,
      [&](data_staging::circle& shape) {
        xy_now = shape.transitive_location_ref().position_cref();
        radius_now = shape.radius() + shape.radius_size();
      },
      [&](data_staging::line& shape) {
        xy_now = shape.transitive_line_start_ref().position_cref();
        xy2_now = shape.transitive_line_end_ref().position_cref();
        compare_xy2 = true;
      },
      [&](data_staging::text& shape) {
        xy_now = shape.transitive_location_ref().position_cref();
      },
      [&](data_staging::script& shape) {
        xy_now = shape.transitive_location_ref().position_cref();
      });

  meta_visit(
      shape_prev,
      [&](data_staging::circle& shape) {
        xy_prev = shape.transitive_location_ref().position_cref();
        radius_prev = shape.radius() + shape.radius_size();
      },
      [&](data_staging::line& shape) {
        xy_prev = shape.transitive_line_start_ref().position_cref();
        xy2_prev = shape.transitive_line_end_ref().position_cref();
      },
      [&](data_staging::text& shape) {
        xy_prev = shape.transitive_location_ref().position_cref();
      },
      [&](data_staging::script& shape) {
        xy_prev = shape.transitive_location_ref().position_cref();
      });

  auto dist = xy_now.distance_to(xy_prev);
  if (compare_xy2) dist = std::max(dist, xy2_now.distance_to(xy2_prev));

  dist += squared_dist(radius_now, radius_prev);

  return dist;
  return 1;  // temp
  //  // Update level for all objects
  //  // TODO: move to better place
  //  auto level = i.integer_number(instance, "level");
  //  auto type = i.str(instance, "type");
  //  auto is_line = type == "line";
  //  auto label = i.str(instance, "label");
  //  auto random_hash = i.str(instance, "__random_hash__");
  //  auto shape_scale = i.has_field(instance, "scale") ? i.double_number(instance, "scale") : 1.0;
  //  auto prev_shape_scale = i.has_field(previous_instance, "scale") ? i.double_number(previous_instance, "scale")
  //  : 1.0;
  //
  //  const auto calculate = [this](v8_interact& i,
  //                                v8::Local<v8::Array>& next_instances,
  //                                v8::Local<v8::Object>& instance,
  //                                std::unordered_map<int64_t, v8::Local<v8::Object>>& lookup,
  //                                int64_t level,
  //                                bool is_line) {
  //    double angle = 0;
  //
  //    data::coord pos;     // X, Y
  //    data::coord pos2;    // for lines there is an X2, Y2.
  //    data::coord parent;  // X, Y of parent
  //                         //    data::coord parent2; // X2, Y2 of parent
  //                         //    data::coord parent3; // centered X,Y of parent (i.o.w., middle of the line for lines)
  //    data::coord pos_for_parent;
  //
  //    std::vector<v8::Local<v8::Object>> lineage;
  //    lineage.push_back(instance);
  //    auto parent_uid = i.integer_number(instance, "parent_uid");
  //    while (parent_uid != -1) {
  //      auto parent = lookup.at(parent_uid);
  //      lineage.push_back(parent);
  //      parent_uid = i.integer_number(parent, "parent_uid");
  //    }
  //
  //    // reverse iterate over lineage vector
  //    for (auto it = lineage.rbegin(); it != lineage.rend(); ++it) {
  //      auto& current_obj = *it;
  //      const bool current_is_line = i.str(current_obj, "type") == "line";
  //      // const bool current_is_pivot =
  //      //    i.has_field(parent, "pivot") ? i.boolean(current_obj, "pivot") : false;
  //
  //      // X,Y
  //      data::coord current{i.double_number(current_obj, "x"), i.double_number(current_obj, "y")};
  //      const double current_angle = i.double_number(current_obj, "angle", 0.);
  //
  //      // X2, Y2, and centerX, centerY, for lines
  //      data::coord current2, current_center;
  //      if (current_is_line) {
  //        current2 = data::coord{i.double_number(current_obj, "x2"), i.double_number(current_obj, "y2")};
  //        current_center.x = ((current.x - current2.x) / 2) + current2.x;
  //        current_center.y = ((current.y - current2.y) / 2) + current2.y;
  //      }
  //
  //      // OPTION 1: simply center from X,Y always (option for points and lines)
  //      // pos.add(current);
  //
  //      // OPTION 2: center from X2, Y2 (only option for lines)
  //      // pos.add(current2);
  //
  //      // OPTION 3: center from centerX, centerY (only option for lines)
  //      // pos.add(current_center);
  //
  //      // for now, let's go for the most intuitive choice
  //      pos_for_parent.add(current_is_line ? current_center : current);
  //      pos.add(current);
  //      pos2.add(current_is_line ? current2 : current);
  //
  //      // angle = current_is_pivot ? current_angle : (angle + current_angle);
  //
  //      // total angle, cumulative no matter what
  //      if (!std::isnan(current_angle)) angle += current_angle;
  //
  //      if (angle != 0.) {
  //        if (current_is_line) {
  //          // current angle + angle with parent
  //          auto angle1 = angle + get_angle(parent.x, parent.y, pos2.x, pos2.y);
  //          while (angle1 > 360.) angle1 -= 360.;
  //          auto rads = angle1 * M_PI / 180.0;
  //          auto ratio = 1.0;
  //          // rotates around its center
  //          auto dist = get_distance(current_center.x, current_center.y, current.x, current.y);
  //          auto move = dist * ratio * -1;
  //          if (false) {
  //            // auto dist = get_distance(current_center.x, current_center.y, current.x, current.y);
  //            // auto move = dist * ratio * -1;
  //            pos.x = parent.x + (cos(rads) * move) + current_center.x;
  //            pos.y = parent.y + (sin(rads) * move) + current_center.y;
  //            pos2.x = parent.x - (cos(rads) * move) + current_center.x;
  //            pos2.y = parent.y - (sin(rads) * move) + current_center.y;
  //          }
  //          // rotates in length
  //          if (true) {
  //            dist = get_distance(current.x, current.y, current2.x, current2.y);
  //            move = dist * ratio * -1;
  //            // pos.x = parent.x + (cos(rads) * move);
  //            // pos.y = parent.y + (sin(rads) * move);
  //            pos2.x = parent.x + (cos(rads) * move);
  //            pos2.y = parent.y + (sin(rads) * move);
  //          }
  //          // unsure
  //          pos_for_parent.x = parent.x + (cos(rads) * move);  // + current_center.x;
  //          pos_for_parent.y = parent.y + (sin(rads) * move);  // + current_center.y;
  //        } else {
  //          // current angle + angle with parent
  //          auto angle1 = angle + get_angle(parent.x, parent.y, pos.x, pos.y);
  //          while (angle1 > 360.) angle1 -= 360.;
  //          auto rads = angle1 * M_PI / 180.0;
  //          auto ratio = 1.0;
  //          auto dist = get_distance(0, 0, current.x, current.y);
  //          auto move = dist * ratio * -1;
  //          pos.x = parent.x + (cos(rads) * move);
  //          pos.y = parent.y + (sin(rads) * move);
  //          pos_for_parent.x = parent.x + (cos(rads) * move);
  //          pos_for_parent.y = parent.y + (sin(rads) * move);
  //        }
  //      }
  //
  //      // now we can update the parent for the next level we're about to handle
  //      parent = pos_for_parent;
  //    }
  //
  //    // store transitive x & y etc.
  //    i.set_field(instance, "transitive_x", v8::Number::New(i.get_isolate(), pos.x));
  //    i.set_field(instance, "transitive_y", v8::Number::New(i.get_isolate(), pos.y));
  //    if (is_line) {
  //      i.set_field(instance, "transitive_x2", v8::Number::New(i.get_isolate(), pos2.x));
  //      i.set_field(instance, "transitive_y2", v8::Number::New(i.get_isolate(), pos2.y));
  //    }
  //
  //    // pass along x, y, x2, y2.
  //    if (i.has_field(instance, "props")) {
  //      const auto process_obj =
  //          [&i, &pos, this](v8::Local<v8::Object>& o, const std::string& inherit_x, const std::string& inherit_y) {
  //            const auto unique_id = i.integer_number(o, "unique_id");
  //            const auto find = next_instance_map.find(unique_id);
  //            if (find != next_instance_map.end()) {
  //              auto other_val = find->second;
  //              if (other_val->IsObject()) {
  //                auto other = other_val.As<v8::Object>();
  //                i.set_field(other, inherit_x, v8::Number::New(i.get_isolate(), pos.x));
  //                i.set_field(other, inherit_y, v8::Number::New(i.get_isolate(), pos.y));
  //              }
  //            }
  //          };
  //      const auto process = [&i, &process_obj](const v8::Local<v8::Value>& field_value,
  //                                              const std::string& inherit_x,
  //                                              const std::string& inherit_y) {
  //        if (field_value->IsArray()) {
  //          auto a = field_value.As<v8::Array>();
  //          for (size_t l = 0; l < a->Length(); l++) {
  //            auto o = i.get_index(a, l).As<v8::Object>();
  //            process_obj(o, inherit_x, inherit_y);
  //          }
  //        } else if (field_value->IsObject()) {
  //          auto o = field_value.As<v8::Object>();
  //          process_obj(o, inherit_x, inherit_y);
  //        }
  //      };
  //      auto props = i.v8_obj(instance, "props");
  //      auto obj_fields = i.prop_names(props);
  //      for (size_t k = 0; k < obj_fields->Length(); k++) {
  //        auto field_name = i.get_index(obj_fields, k);
  //        auto field_value = i.get(props, field_name);
  //        if (!field_value->IsObject()) continue;
  //        auto str = i.str(obj_fields, k);
  //        if (str == "left") {
  //          process(field_value, "inherited_x", "inherited_y");
  //        } else if (str == "right") {
  //          process(field_value, "inherited_x2", "inherited_y2");
  //        }
  //      }
  //    }
  //
  //    return std::make_tuple(pos.x, pos.y, pos2.x, pos2.y);
  //  };
  //
  //  auto [x, y, x2, y2] = calculate(i, next_instances, instance, next_instance_map, level, is_line);
  //  auto [prev_x, prev_y, prev_x2, prev_y2] =
  //      calculate(i, next_instances, previous_instance, intermediate_map, level, is_line);
  //
  //  // Calculate how many pixels are maximally covered by this instance, this is currently very simplified
  //  x *= scalesettings.video_scale_next * shape_scale;
  //  prev_x *= scalesettings.video_scale_intermediate * prev_shape_scale;
  //  y *= scalesettings.video_scale_next * shape_scale;
  //  prev_y *= scalesettings.video_scale_intermediate * prev_shape_scale;
  //
  //  //  // TODO: make smarter for circles and lines, this is just temporary code
  //  //  rectangle canvas(position(-canvas_w / 2., -canvas_h/2.), position(canvas_w / 2., canvas_h/2.));
  //  //  if (is_line) {
  //  //    rectangle shape_rect(position(x, y), position(x2, y2));
  //  //    rectangle prev_shape_rect(position(prev_x - rad, prev_y - rad), position(prev_x + rad, prev_y + rad));
  //  //    if (!canvas.overlaps(shape_rect) && !canvas.overlaps(prev_shape_rect)) {
  //  //      return 0.;
  //  //    }
  //  //  } else {
  //  //    rectangle shape_rect(position(x - rad, y - rad), position(x + rad, y + rad));
  //  //    rectangle prev_shape_rect(position(prev_x - rad, prev_y - rad), position(prev_x + rad, prev_y + rad));
  //  //    if (!canvas.overlaps(shape_rect) && !canvas.overlaps(prev_shape_rect)) {
  //  //      return 0.;
  //  //    }
  //  //  }
  //  auto dist = sqrt(pow(x - prev_x, 2) + pow(y - prev_y, 2));
  //  // x2, y2
  //  if (is_line) {
  //    x2 *= scalesettings.video_scale_next * shape_scale;
  //    prev_x2 *= scalesettings.video_scale_intermediate * prev_shape_scale;
  //    y2 *= scalesettings.video_scale_next * shape_scale;
  //    prev_y2 *= scalesettings.video_scale_intermediate * prev_shape_scale;
  //    dist = std::max(dist, sqrt(pow(x2 - prev_x2, 2) + pow(y2 - prev_y2, 2)));
  //  }
  //
  //  // radius
  //  auto radius = i.double_number(instance, "radius");
  //  auto radiussize = i.double_number(instance, "radiussize");
  //  auto prev_radius = i.double_number(previous_instance, "radius");
  //  auto prev_radiussize = i.double_number(previous_instance, "radiussize");
  //  auto rad = radius + radiussize;
  //  auto prev_rad = prev_radius + prev_radiussize;
  //  rad *= scalesettings.video_scale_next * shape_scale;
  //  prev_rad *= scalesettings.video_scale_intermediate * prev_shape_scale;
  //  // TODO: this is not 100% accurate, if an object is moving and expanding its radius the max distance can be more
  //  // but this should at least result in a decent enough effect in most cases
  //  dist = std::max(dist, fabs(prev_rad - rad));
  //
  //  // Make sure that we do not include any warped distance
  // This is now in toroidal_ref()
  //  if (i.has_field(instance, "__warped_dist__")) {
  //    dist -= i.double_number(instance, "__warped_dist__");
  //    // If object moves two pixels to the right, is then warped 500 px to the left
  //    // It has traveled 498, and the above statement has resulted in a value of -2.
  //    // If it moves to the left for 2 pixels, and is moved 500px to the right, the
  //    // distance is 502px - 500px = 2px as well.
  //    dist = fabs(dist);
  //  }
  //
  //  // TODO: proper support for connected objects, so they can inherit the warped stuff!
  //  if (dist > 100) {
  //    // logger(INFO) << "Warning, distance found > 100, id hash: " << random_hash << std::endl;
  //  }
  //  return dist;
}

void native_generator::convert_objects_to_render_job(v8_interact& i, step_calculator& sc, v8::Local<v8::Object> video) {
  //  // Risking doing this for nothing, as this may still be discarded, we'll translate all the instances to
  //  // objects ready for rendering Note that we save object states for multiple "steps" per frame if needed.
  //  for (size_t index = 0; index < next_instances->Length(); index++) {
  //    auto instance = i.get_index(next_instances, index).As<v8::Object>();
  //    if (!instance->IsObject()) continue;
  //    convert_object_to_render_job(i, instance, index, sc, video);
  //  }
  for (auto& shape : scene_shapes_next[scenesettings.current_scene_next]) {
    convert_object_to_render_job(i, shape, sc, video);
  }
}

void native_generator::convert_object_to_render_job(v8_interact& i,
                                                    data_staging::shape_t& shape,
                                                    step_calculator& sc,
                                                    v8::Local<v8::Object> video) {
  data::shape new_shape;

  const auto initialize = [&]<typename T>(T& shape) {
    auto level = 0;      // shape.level;
    auto exists = true;  // !i.has_field(instance, "exists") || i.boolean(instance, "exists");
    if (!exists) return;
    // See if we require this step for this object
    // auto steps = i.integer_number(instance, "steps");
    // if (minimize_steps_per_object && !sc.do_step(steps, stepper.next_step)) {
    // TODO: make this a property also for objects, if they are vibrating they need this
    //  return;
    //}
    // auto id = i.str(instance, "id");
    // auto label = i.str(instance, "label");
    // auto time = i.double_number(instance, "__time__");

    // auto radius = shape.radius();           // i.double_number(instance, "radius");
    // auto radiussize = shape.radius_size();  // i.double_number(instance, "radiussize");
    // auto seed = i.double_number(instance, "seed");
    auto scale = shape.generic_cref().scale();

    auto shape_opacity = shape.generic_cref().opacity();
    auto warp_width = shape.toroidal_ref().warp_width();
    auto warp_height = shape.toroidal_ref().warp_height();

    // auto text_font = i.has_field(instance, "text_font") ? i.str(instance, "text_font") : "";

    // TODO: might not need this param after all
// auto dist = i.double_number(instance, "__dist__");
#define DEBUG_NUM_SHAPES
#ifdef DEBUG_NUM_SHAPES
// auto random_hash = i.str(instance, "__random_hash__");
#endif

    // temp
    new_shape.level = level;
    new_shape.time = shape.meta_cref().get_time();
    // new_shape.dist = dist;

    new_shape.gradients_.clear();
    new_shape.textures.clear();
    std::string gradient_id_str;

    copy_gradient_from_object_to_shape(shape, new_shape, gradients);
    copy_texture_from_object_to_shape(shape, new_shape, textures);

    // temp hack
    std::string namespace_ = "";
    std::string gradient_id;
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      gradient_id = shape.styling_cref().gradient();
    }

    if (!gradient_id.empty()) {
      if (new_shape.gradients_.empty()) {
        auto& known_gradients_map = gradients;
        if (known_gradients_map.contains(gradient_id)) {
          new_shape.gradients_.emplace_back(1.0, known_gradients_map[gradient_id]);
        }
      }
    }

    if (new_shape.gradients_.empty()) {
      new_shape.gradients_.emplace_back(1.0, data::gradient{});
      new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
      new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
      new_shape.gradients_[0].second.colors.emplace_back(1.0, data::color{0.0, 0, 0, 1});
    }
    new_shape.z = 0;
    // new_shape.vel_x = vel_x;
    // new_shape.vel_y = vel_y;
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      new_shape.blending_ = shape.styling_cref().blending_type();
    }
    new_shape.scale = scale;
    new_shape.opacity = std::isnan(shape_opacity) ? 1.0 : shape_opacity;
    // new_shape.unique_id = unique_id;
#ifdef DEBUG_NUM_SHAPES
    // new_shape.random_hash = random_hash;
#endif
    new_shape.seed = seed;
    new_shape.id = shape.meta_cref().id();
    // new_shape.label = label;
    // new_shape.motion_blur = motion_blur;
    new_shape.warp_width = warp_width;
    new_shape.warp_height = warp_height;

    // }
    // wrap this in a proper add method
    if (stepper.next_step != stepper.max_step) {
      indexes[shape.meta_cref().unique_id()][stepper.current_step] = job->shapes[stepper.current_step].size();
    } else {
      new_shape.indexes = indexes[shape.meta_cref().unique_id()];
    }
    // logger(INFO) << "sizeof shape: " << sizeof(new_shape) << " circle was size: " << sizeof(shape) <<
    // std::endl;
    //                   if (job->shapes[stepper.current_step].capacity() < 10000) {
    //                     logger(INFO) << "resizing to fix capacity" << std::endl;
    //                     job->shapes[stepper.current_step].reserve(10000);
    //                   }
    // why is this shit super slow
    job->shapes[stepper.current_step].emplace_back(std::move(new_shape));
    // and this reasonably fast
    // job->shapes_prototype_test[stepper.current_step].emplace_back(shape);
    // job->shapes[stepper.current_step].emplace_back(data::shape{});
    //                   if (job->shapes[stepper.current_step].size() > 9999)
    //                     logger(INFO) << "current_step = " << stepper.current_step << ", shapes: " <<
    //                     job->shapes[stepper.current_step].size() << std::endl;
    job->scale = scalesettings.video_scale;
    job->scales = scalesettings.video_scales;
  };

  // Update level for all objects
  meta_visit(
      shape,
      [&](data_staging::circle& shape) {
        new_shape.type = data::shape_type::circle;
        new_shape.radius = shape.radius();
        new_shape.radius_size = shape.radius_size();
        new_shape.x = shape.transitive_location_ref().position_cref().x;
        new_shape.y = shape.transitive_location_ref().position_cref().y;
        initialize(shape);
      },
      [&](data_staging::line& shape) {
        new_shape.type = data::shape_type::line;
        new_shape.radius = 0;
        new_shape.radius_size = shape.line_width();
        new_shape.x = shape.transitive_line_start_ref().position_cref().x;
        new_shape.y = shape.transitive_line_start_ref().position_cref().y;
        new_shape.x2 = shape.transitive_line_end_ref().position_cref().x;
        new_shape.y2 = shape.transitive_line_end_ref().position_cref().y;
        initialize(shape);
      },
      [&](data_staging::text& shape) {
        new_shape.type = data::shape_type::text;
        new_shape.text = shape.text_cref();
        new_shape.text_size = shape.text_size();
        new_shape.align = shape.text_align();
        new_shape.text_fixed = shape.text_fixed();
        // TODO: new_shape.text_font = shape.text_font();
        new_shape.x = shape.transitive_location_ref().position_cref().x;
        new_shape.y = shape.transitive_location_ref().position_cref().y;
        initialize(shape);
      },
      [&](data_staging::script& shape) {
        new_shape.type = data::shape_type::script;
        new_shape.x = shape.transitive_location_ref().position_cref().x;
        new_shape.y = shape.transitive_location_ref().position_cref().y;
        initialize(shape);
      });
}

std::shared_ptr<data::job> native_generator::get_job() const {
  return job;
}

int64_t native_generator::spawn_object(data_staging::shape_t& spawner, v8::Local<v8::Object> obj) {
  auto& i = genctx->i();
  auto [created_instance, shape_ref, created_shape_copy] = _instantiate_object_from_scene(i, obj, &spawner);
  create_bookkeeping_for_script_objects(created_instance, created_shape_copy);
  return i.integer_number(created_instance, "unique_id");
}

int64_t native_generator::spawn_object3(data_staging::shape_t& spawner,
                                        v8::Local<v8::Object> line_obj,
                                        int64_t obj1,
                                        int64_t obj2) {
  auto& i = genctx->i();
  auto [created_instance, shape_ref, created_shape_copy] = _instantiate_object_from_scene(i, line_obj, &spawner);
  // BEGIN: Temporary code (to try out something
  data_staging::shape_t* obj1o = nullptr;
  data_staging::shape_t* obj2o = nullptr;
  auto find1 = next_instance_map.find(obj1);
  if (find1 != next_instance_map.end()) {
    obj1o = &find1->second.get();
  } else {
    // todo; we need a mapping for this...
    for (auto& newo : instantiated_objects[scenesettings.current_scene_next]) {
      if (auto c = std::get_if<data_staging::circle>(&newo)) {
        if (c->meta_cref().unique_id() == obj1) {
          obj1o = &newo;
        }
      }
    }
  }
  auto find2 = next_instance_map.find(obj2);
  if (find2 != next_instance_map.end()) {
    obj2o = &find2->second.get();
  } else {
    // todo; we need a mapping for this...
    for (auto& newo : instantiated_objects[scenesettings.current_scene_next]) {
      if (auto c = std::get_if<data_staging::circle>(&newo)) {
        if (c->meta_cref().unique_id() == obj2) {
          obj2o = &newo;
        }
      }
    }
  }
  // END
  const auto handle = [](auto& line_o, auto& side_a, auto& side_b) {
    try {
      auto& line_obj = std::get<data_staging::line>(line_o);
      auto& circle_obj1 = std::get<data_staging::circle>(side_a);
      auto& circle_obj2 = std::get<data_staging::circle>(side_b);
      circle_obj1.add_cascade_out(cascade_type::start, line_obj.meta_cref().unique_id());
      circle_obj2.add_cascade_out(cascade_type::end, line_obj.meta_cref().unique_id());
      line_obj.add_cascade_in(cascade_type::start, circle_obj1.meta_cref().unique_id());
      line_obj.add_cascade_in(cascade_type::end, circle_obj2.meta_cref().unique_id());
    } catch (std::bad_variant_access const& ex) {
      return;
    }
  };
  handle(shape_ref.get(), *obj1o, *obj2o);
  create_bookkeeping_for_script_objects(created_instance, created_shape_copy);
  return i.integer_number(created_instance, "unique_id");
}

std::unordered_map<std::string, v8::Persistent<v8::Object>>& native_generator::get_object_definitions_ref() {
  return object_definitions_map;
}

// TODO: will refactor soon
void native_generator::fix_xy(v8_interact& i, v8::Local<v8::Object>& instance, int64_t uid, double& x, double& y) {
  //  // experimental function caching
  //  double xx = 0;
  //  double yy = 0;
  //  auto find = cached_xy.find(uid);
  //  if (find != cached_xy.end()) {
  //    std::tie(xx, yy) = find->second;
  //    x += xx;
  //    y += yy;
  //    return;
  //  }
  //  auto parent_uid = i.integer_number(instance, "parent_uid", -1);
  //  while (parent_uid != -1) {
  //    auto parent = next_instance_map.at(parent_uid);
  //    if (i.str(parent, "type", "") == "script") {
  //      xx += i.double_number(parent, "x");
  //      yy += i.double_number(parent, "y");
  //    }
  //    parent_uid = i.integer_number(parent, "parent_uid");
  //  }
  //  cached_xy[uid] = std::make_pair(xx, yy);
  //  x += xx;
  //  y += yy;
}

std::tuple<v8::Local<v8::Object>, std::reference_wrapper<data_staging::shape_t>, data_staging::shape_t>
native_generator::_instantiate_object_from_scene(
    v8_interact& i,
    v8::Local<v8::Object>& scene_object,        // object description from scene to be instantiated
    const data_staging::shape_t* parent_object  // it's optional parent
) {
  v8::Isolate* isolate = i.get_isolate();

  int64_t current_level = 0;
  auto parent_object_ns = std::string("");
  int64_t parent_uid = -1;

  if (parent_object) {
    meta_callback(*parent_object, [&]<typename T>(const T& cc) {
      current_level = cc.meta_cref().level() + 1;
      parent_object_ns = cc.meta_cref().namespace_name();
      parent_uid = cc.meta_cref().unique_id();
    });
  }

  // lookup the object prototype to be instantiated
  auto object_id = parent_object_ns + i.str(scene_object, "id", "");
  auto object_prototype = v8_index_object(i.get_context(), genctx->objects, object_id).template As<v8::Object>();

  // create a new javascript object
  v8::Local<v8::Object> instance = v8::Object::New(isolate);

  // TODO: make sure this is the only source..., and get rid of genctx->objects usage
  if (!object_prototype->IsObject()) {
    object_prototype = object_definitions_map[object_id].Get(isolate);
  }

  if (!object_prototype->IsObject()) {
    logger(WARNING) << "cannot instantiate object id: " << object_id << ", does not exist" << std::endl;
    throw std::runtime_error(fmt::format("cannot instantiate object id: {}, does not exist", object_id));
  }

  // instantiate the prototype into newly allocated javascript object
  _instantiate_object(i, scene_object, object_prototype, instance, current_level, parent_object_ns);

  // give it a unique id (it already has been assigned a __random_hash__ for debugging purposes
  static int64_t counter = 0;
  i.set_field(instance, "unique_id", v8::Number::New(i.get_isolate(), ++counter));
  i.set_field(instance, "parent_uid", v8::Number::New(i.get_isolate(), parent_uid));

  // TODO: in the future we will simply instantiate this directly, for now, to save some refactoring time
  // we will map, to see if the proof of concept works

  // TODO: try to use a raw pointer, see if it improves performance
  std::optional<std::reference_wrapper<data_staging::shape_t>> shape_ref;
  std::optional<data_staging::shape_t> shape_copy;

  const auto handle = [&]<typename T>(T& c) -> data_staging::shape_t& {
    // we buffer instantiated objects, and will insert in the array later.
    instantiated_objects[scenesettings.current_scene_next].emplace_back(c);
    return instantiated_objects[scenesettings.current_scene_next].back();
  };

  const auto type = i.str(object_definitions_map[object_id], "type", "");

  const auto initialize = [&]<typename T>(T& c, auto& bridge) {
    c.meta_ref().set_level(current_level);
    c.meta_ref().set_parent_uid(parent_uid);
    c.meta_ref().set_pivot(i.boolean(instance, "pivot"));
    c.behavior_ref().set_collision_group(i.str(instance, "collision_group", ""));
    c.behavior_ref().set_gravity_group(i.str(instance, "gravity_group", ""));
    c.toroidal_ref().set_group(i.str(instance, "toroidal", ""));
    c.generic_ref().set_angle(i.double_number(instance, "angle", 0));
    c.generic_ref().set_opacity(i.double_number(instance, "opacity", 1));
    c.generic_ref().set_mass(i.double_number(instance, "mass", 1));
    c.generic_ref().set_scale(i.double_number(instance, "scale", 1));
    if constexpr (!std::is_same_v<T, data_staging::script>) {
      if (i.has_field(instance, "texture")) {
        c.styling_ref().set_texture(i.str(instance, "texture"));
      }
      if (i.has_field(instance, "gradient")) {
        c.styling_ref().set_gradient(i.str(instance, "gradient"));
      }
    }
    if (i.has_field(instance, "gradients")) {
      auto gradient_array = i.v8_array(instance, "gradients");
      for (size_t k = 0; k < gradient_array->Length(); k++) {
        auto gradient_data = i.get_index(gradient_array, k).As<v8::Array>();
        if (!gradient_data->IsArray()) continue;
        auto opacity = i.double_number(gradient_data, size_t(0));
        auto gradient_id = parent_object_ns + i.str(gradient_data, size_t(1));
        if constexpr (!std::is_same_v<T, data_staging::script>) {
          c.styling_ref().add_gradient(opacity, gradient_id);
        }
      }
    }
    if (i.has_field(instance, "props")) {
      auto props_object = i.v8_obj(instance, "props");
      auto obj_fields = i.prop_names(props_object);
      for (size_t k = 0; k < obj_fields->Length(); k++) {
        auto field_name = i.get_index(obj_fields, k);
        auto field_name_str = i.str(obj_fields, k);
        auto field_value = i.get(props_object, field_name);
        i.set_field(c.properties_ref().properties_ref(), field_name, field_value);
      }
    }

    // the handle function returns a ref, which is all fine, but recursively init
    // may actually invalidate this ref with other inserts.
    shape_ref = std::ref(handle(c));
    shape_copy = (*shape_ref).get();

    // call init last, so that objects exist when using 'this' inside init()
    if (bridge) {
      // take a copy as the reference might point to a non-existant instance at some point,
      // for example when other cascading 'init's insert new objects.
      auto copy = std::get<T>((*shape_ref).get());
      bridge->push_object(copy);
      i.call_fun(object_definitions_map[object_id],  // object definition
                 bridge->instance(),                 // bridged object is "this"
                 "init");
      bridge->pop_object();
      write_back_copy(copy);
    }
  };

  if (type == "circle" || type.empty() /* treat every "non-type" as circles too */) {
    data_staging::circle c(object_id,
                           counter,
                           vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                           i.double_number(instance, "radius"),
                           i.double_number(instance, "radiussize"));

    if (type.empty()) c.generic_ref().set_opacity(0);

    c.movement_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                  i.double_number(instance, "vel_y", 0),
                                  i.double_number(instance, "velocity", 0));

    c.meta_ref().set_namespace(parent_object_ns);

    initialize(c, object_bridge_circle);

  } else if (type == "line") {
    data_staging::line c(object_id,
                         counter,
                         vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                         vector2d(i.double_number(instance, "x2"), i.double_number(instance, "y2")),
                         i.double_number(instance, "radiussize"));

    // TODO: no logic for end of line
    c.movement_line_start_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                             i.double_number(instance, "vel_y", 0),
                                             i.double_number(instance, "velocity", 0));

    c.meta_ref().set_namespace(parent_object_ns);

    initialize(c, object_bridge_line);
  } else if (type == "text") {
    data_staging::text t(object_id,
                         counter,
                         vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")),
                         i.str(instance, "text"),
                         i.double_number(instance, "text_size"),
                         i.str(instance, "text_align"),
                         i.boolean(instance, "text_fixed"));

    t.movement_ref().set_velocity(i.double_number(instance, "vel_x", 0),
                                  i.double_number(instance, "vel_y", 0),
                                  i.double_number(instance, "velocity", 0));

    t.meta_ref().set_namespace(parent_object_ns);

    initialize(t, object_bridge_text);

  } else if (type == "script") {
    data_staging::script s(
        object_id, counter, vector2d(i.double_number(instance, "x"), i.double_number(instance, "y")));

    s.meta_ref().set_namespace(object_id + "_");

    initialize(s, object_bridge_script);
  } else {
    throw std::logic_error(fmt::format("unknown type: {}", type));
  }

  if (!shape_ref) {
    throw std::runtime_error("unexpected shape_ref not set to reference");
  }

  return std::make_tuple(instance, *shape_ref, *shape_copy);
}
void native_generator::_instantiate_object(v8_interact& i,
                                           std::optional<v8::Local<v8::Object>> scene_obj,
                                           v8::Local<v8::Object> object_prototype,
                                           v8::Local<v8::Object> new_instance,
                                           int64_t level,
                                           const std::string& namespace_) {
  v8::Isolate* isolate = i.get_isolate();

  i.recursively_copy_object(new_instance, object_prototype);

  if (!namespace_.empty()) {
    i.set_field(new_instance, "namespace", v8_str(i.get_context(), namespace_));
  }

  if (scene_obj) {
    i.copy_field_if_exists(new_instance, "id", *scene_obj);
    i.copy_field_if_exists(new_instance, "x", *scene_obj);
    i.copy_field_if_exists(new_instance, "y", *scene_obj);
    i.copy_field_if_exists(new_instance, "x2", *scene_obj);
    i.copy_field_if_exists(new_instance, "y2", *scene_obj);
    i.copy_field_if_exists(new_instance, "vel_x", *scene_obj);
    i.copy_field_if_exists(new_instance, "vel_y", *scene_obj);
    i.copy_field_if_exists(new_instance, "vel_x2", *scene_obj);
    i.copy_field_if_exists(new_instance, "vel_y2", *scene_obj);
    i.copy_field_if_exists(new_instance, "velocity", *scene_obj);
    i.copy_field_if_exists(new_instance, "mass", *scene_obj);
    i.copy_field_if_exists(new_instance, "radius", *scene_obj);
    i.copy_field_if_exists(new_instance, "radiussize", *scene_obj);
    i.copy_field_if_exists(new_instance, "gradient", *scene_obj);
    i.copy_field_if_exists(new_instance, "texture", *scene_obj);
    i.copy_field_if_exists(new_instance, "seed", *scene_obj);
    i.copy_field_if_exists(new_instance, "blending_type", *scene_obj);
    i.copy_field_if_exists(new_instance, "opacity", *scene_obj);
    i.copy_field_if_exists(new_instance, "scale", *scene_obj);
    i.copy_field_if_exists(new_instance, "angle", *scene_obj);
    i.copy_field_if_exists(new_instance, "pivot", *scene_obj);
    i.copy_field_if_exists(new_instance, "text", *scene_obj);
    i.copy_field_if_exists(new_instance, "text_align", *scene_obj);
    i.copy_field_if_exists(new_instance, "text_size", *scene_obj);
    i.copy_field_if_exists(new_instance, "text_fixed", *scene_obj);
    i.copy_field_if_exists(new_instance, "text_font", *scene_obj);
    i.copy_field_if_exists(new_instance, "file", *scene_obj);
    i.copy_field_if_exists(new_instance, "duration", *scene_obj);
  }

  i.set_field(new_instance, "subobj", v8::Array::New(isolate));
  i.set_field(new_instance, "level", v8::Number::New(isolate, level));
  {
    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_int_distribution<int> distribution{'a', 'z'};
    static auto generate_len = 6;
    static std::string rand_str(generate_len, '\0');
    for (auto& dis : rand_str) dis = distribution(generator);
    i.set_field(new_instance, "__random_hash__", v8_str(i.get_context(), rand_str));
  }
  i.set_field(new_instance, "__instance__", v8::Boolean::New(isolate, true));

  // Make sure we deep copy the gradients
  i.set_field(new_instance, "gradients", v8::Array::New(isolate));
  auto dest_gradients = i.get(new_instance, "gradients").As<v8::Array>();
  auto gradients = i.has_field(object_prototype, "gradients") && i.get(object_prototype, "gradients")->IsArray()
                       ? i.get(object_prototype, "gradients").As<v8::Array>()
                       : v8::Array::New(i.get_isolate());
  for (size_t k = 0; k < gradients->Length(); k++) {
    i.set_field(dest_gradients, k, v8::Array::New(isolate));

    auto gradient = i.get_index(gradients, k).As<v8::Array>();
    auto dest_gradient = i.get_index(dest_gradients, k).As<v8::Array>();
    for (size_t l = 0; l < gradient->Length(); l++) {
      i.set_field(dest_gradient, l, i.get_index(gradient, l));
    }
  }

  // Ensure we have a props object in the new obj
  if (!i.has_field(new_instance, "props")) {
    i.set_field(new_instance, "props", v8::Object::New(i.get_isolate()));
  }

  // Copy over scene properties to instance properties
  if (scene_obj) {
    auto props = i.v8_obj(new_instance, "props");
    auto scene_props = i.v8_obj(*scene_obj, "props");
    auto obj_fields = i.prop_names(scene_props);

    for (size_t k = 0; k < obj_fields->Length(); k++) {
      auto field_name = i.get_index(obj_fields, k);
      auto field_value = i.get(scene_props, field_name);
      i.set_field(props, field_name, field_value);
    }
  }

  auto the_fun = i.get_fun("__spawn__");
  i.set_fun(new_instance, "spawn", the_fun);
}

void native_generator::debug_print_next(const std::string& desc) {
  logger(INFO) << "desc = " << desc << std::endl;
  const auto print_meta = [](data_staging::meta& meta,
                             data_staging::location& loc,
                             data_staging::movement& mov,
                             data_staging::behavior& beh,
                             data_staging::generic& gen) {
    logger(INFO) << "uid=" << meta.unique_id() << ", puid=" << meta.parent_uid() << ", id=" << meta.id()
                 << ", level=" << meta.level() << ", namespace=" << meta.namespace_name() << " @ "
                 << loc.position_cref().x << "," << loc.position_cref().y << " +" << mov.velocity().x << ","
                 << mov.velocity().y << ", last_collide=" << beh.last_collide() << ", mass=" << gen.mass() << std::endl;
  };
  for (auto& shape : scene_shapes_next[scenesettings.current_scene_next]) {
    meta_visit(
        shape,
        [&](data_staging::circle& shape) {
          print_meta(
              shape.meta_ref(), shape.location_ref(), shape.movement_ref(), shape.behavior_ref(), shape.generic_ref());
        },
        [&](data_staging::line& shape) {
          print_meta(shape.meta_ref(),
                     shape.line_start_ref(),
                     shape.movement_line_start_ref(),
                     shape.behavior_ref(),
                     shape.generic_ref());
        },
        [&](data_staging::text& shape) {
          print_meta(
              shape.meta_ref(), shape.location_ref(), shape.movement_ref(), shape.behavior_ref(), shape.generic_ref());
        },
        [&](data_staging::script& shape) {
          print_meta(
              shape.meta_ref(), shape.location_ref(), shape.movement_ref(), shape.behavior_ref(), shape.generic_ref());
        });
  }
}

template <typename T>
void native_generator::copy_gradient_from_object_to_shape(
    T& source_object,
    data::shape& destination_shape,
    std::unordered_map<std::string, data::gradient>& known_gradients_map) {
  if constexpr (!std::is_same_v<T, data_staging::script>) {
    std::string namespace_ = source_object.meta_cref().namespace_name();
    std::string gradient_id = namespace_ + source_object.styling_ref().gradient();

    if (!gradient_id.empty()) {
      if (destination_shape.gradients_.empty()) {
        if (known_gradients_map.find(gradient_id) != known_gradients_map.end()) {
          destination_shape.gradients_.emplace_back(1.0, known_gradients_map[gradient_id]);
        }
      }
    }
    for (const auto& [opacity, gradient_id] : source_object.styling_ref().get_gradients_cref()) {
      destination_shape.gradients_.emplace_back(opacity, known_gradients_map[gradient_id]);
    }
  }
}

template <typename T>
void native_generator::copy_texture_from_object_to_shape(
    T& source_object,
    data::shape& destination_shape,
    std::unordered_map<std::string, data::texture>& known_textures_map) {
  if constexpr (!std::is_same_v<T, data_staging::script>) {
    std::string namespace_ = source_object.meta_cref().namespace_name();
    std::string texture_id = namespace_ + source_object.styling_ref().texture();

    if (!texture_id.empty()) {
      if (destination_shape.textures.empty()) {
        if (known_textures_map.find(texture_id) != known_textures_map.end()) {
          destination_shape.textures.emplace_back(1.0, known_textures_map[texture_id]);
        }
      }
    }

    for (const auto& [opacity, texture_id] : source_object.styling_ref().get_textures_cref()) {
      destination_shape.textures.emplace_back(opacity, known_textures_map[texture_id]);
    }
  }
}

template <typename T>
void native_generator::write_back_copy(T& copy) {
  size_t index = 0;
  for (auto& instance : instantiated_objects[scenesettings.current_scene_next]) {
    meta_callback(instance, [&]<typename TS>(TS& shape) {
      if (shape.meta_cref().unique_id() == copy.meta_cref().unique_id()) {
        instantiated_objects[scenesettings.current_scene_next][index] = copy;
      }
    });
    index++;
  }
}
