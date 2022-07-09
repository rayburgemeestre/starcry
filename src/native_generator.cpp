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

native_generator::native_generator(std::shared_ptr<metrics>& metrics, std::shared_ptr<v8_wrapper>& context)
    : context(context), metrics_(metrics) {}

native_generator* global_native_generator = nullptr;

v8::Local<v8::Object> spawn_object_native(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto ctx = global_native_generator->get_context();
  v8::Isolate* isolate = ctx->isolate;
  v8::HandleScope scope(isolate);
  v8::Local<v8::Object> obj = args[0]->ToObject(ctx->context->impl()).ToLocalChecked();
  v8_interact i(obj->GetIsolate());
  auto spawner = args.Holder();
  return global_native_generator->spawn_object_native(spawner, obj);
}

void native_generator::reset_context() {
  // reset context
  context->reset();

  // add context global functions
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

  global_native_generator = this;
  context->add_fun("__spawn__", &::spawn_object_native);
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
  prctl(PR_SET_NAME, "generator thread");
  filename_ = filename;
  init_context();
  init_user_script();
  init_job();
  init_video_meta_info(rand_seed, preview);
  init_gradients();
  init_textures();
  init_toroidals();
  scenesettings.scene_initialized = std::numeric_limits<size_t>::max();

  // set_scene requires generator_context to be set
  context->run_array("script", [this](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    genctx = native_generator_context(isolate, val, 0);

    // refresh the scene object to get rid of left-over state
    scene_settings tmp;
    std::swap(scenesettings, tmp);

    // restore scene durations info in the recreated object
    scenesettings.scene_durations = tmp.scene_durations;
    scenesettings.scenes_duration = tmp.scenes_duration;

    // throw away all the scene information for script objects
    scenesettings_objs.clear();

    set_scene(0);
  });
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
    v8_interact i(isolate);
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
    v8_interact i(isolate);
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
    v8_interact i(isolate);
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
    v8_interact i(isolate);
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
  context->run_array("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    genctx.set_scene(scenesettings.current_scene_next);
    auto& i = genctx.i();

    // whenever we switch to a new scene, we'll copy all the object state from the previous scene
    if (scenesettings.current_scene_next > 0) {
      logger(INFO) << "Switching to new scene, copying all state from previous." << std::endl;
      auto prev_current_scene = i.get_index(genctx.scenes, scenesettings.current_scene_next - 1);
      auto prev_sceneobj = prev_current_scene.As<v8::Object>();
      // continue from previous
      //      genctx.instances = i.v8_array(prev_sceneobj, "instances", v8::Array::New(isolate));
      //      genctx.instances_next = i.v8_array(prev_sceneobj, "instances_next", v8::Array::New(isolate));
      //      genctx.instances_intermediate = i.v8_array(prev_sceneobj, "instances_intermediate",
      //      v8::Array::New(isolate)); i.set_field(genctx.current_scene_obj, "instances", genctx.instances);
      //      i.set_field(genctx.current_scene_obj, "instances_next", genctx.instances_next);
      //      i.set_field(genctx.current_scene_obj, "instances_intermediate", genctx.instances_intermediate);
      scene_shapes[scenesettings.current_scene_next] = scene_shapes[scenesettings.current_scene_next - 1];
      scene_shapes_next[scenesettings.current_scene_next] = scene_shapes_next[scenesettings.current_scene_next - 1];
      scene_shapes_intermediate[scenesettings.current_scene_next] =
          scene_shapes_intermediate[scenesettings.current_scene_next - 1];
    }

    instantiate_additional_objects_from_new_scene(genctx.scene_objects);

    // since this is invoked directly after a scene change, and in the very beginning, make sure this state is part of
    // the instances "current" frame, or reverting (e.g., due to motion blur requirements) will discard all of this.
    ////util::generator::copy_instances(i, genctx.instances, genctx.instances_next);
    scene_shapes = scene_shapes_next;
  });
}

void native_generator::create_bookkeeping_for_script_objects(v8::Local<v8::Object> created_instance) {
  auto& i = genctx.i();

  // only do extra work for script objects
  if (i.str(created_instance, "type") != "script") {
    return;
  }

  const auto unique_id = i.integer_number(created_instance, "unique_id");
  const auto namespace_ = i.str(created_instance, "id") + "_";
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
    i.set_field(genctx.gradients, gradient_dst_id, i.get(gradients, gradient_src_id));
  }
  init_gradients();

  // import all object definitions from script
  auto objects = i.v8_obj(tmp, "objects");
  auto objects_fields = objects->GetOwnPropertyNames(i.get_context()).ToLocalChecked();
  for (size_t k = 0; k < objects_fields->Length(); k++) {
    auto object_src_id = i.get_index(objects_fields, k);
    auto object_dst_id = namespace_ + i.str(objects_fields, k);
    i.set_field(genctx.objects, object_dst_id, i.get(objects, object_src_id));
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
    instantiate_additional_objects_from_new_scene(scene_objects, &created_instance);
  }

  // clean-up temporary variable that referenced the entire imported script
  context->run("tmp = undefined;");
}

void native_generator::instantiate_additional_objects_from_new_scene(v8::Local<v8::Array>& scene_objects,
                                                                     v8::Local<v8::Object>* parent_object) {
  auto& i = genctx.i();

  // instantiate all the additional objects from the new scene
  for (size_t j = 0; j < scene_objects->Length(); j++) {
    auto scene_obj = i.get_index(scene_objects, j).As<v8::Object>();
    // below can recursively add new objects as init() invocations spawn new objects, and so on

    static util::random_generator rng;
    static int64_t unique_id = 1;

    logger(INFO) << "Instantiating 10.000 objects.." << std::endl;
    for (int i = 0; i < 10000; i++) {
      data_staging::circle c(unique_id++, vector2d(rng.get() * 1000 - 500, rng.get() * 1000 - 500), 0, 5);
      c.set_velocity(rng.get(), rng.get(), 10.);
      c.set_gradient("blue");
      scene_shapes_next[scenesettings.current_scene_next].push_back(c);
    }

    //    v8::Local<v8::Object> created_instance = util::generator::instantiate_object_from_scene(
    //        i, genctx.objects, genctx.instances_next, scene_obj, parent_object);
    //    create_bookkeeping_for_script_objects(created_instance);
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
      genctx = native_generator_context(isolate, val, scenesettings.current_scene_next);
      auto& i = genctx.i();

      auto obj = val.As<v8::Object>();
      auto scenes = i.v8_array(obj, "scenes");
      auto video = i.v8_obj(obj, "video");
      // auto objects = i.v8_array(obj, "objects");
      auto current_scene = i.get_index(scenes, scenesettings.current_scene_next);
      if (!current_scene->IsObject()) return;
      auto sceneobj = current_scene.As<v8::Object>();

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
          logger(DEBUG) << "Stepper at step " << stepper.current_step << " out of " << stepper.max_step << std::endl;
          qts.clear();
          qts_gravity.clear();

          // initialize scene
          if (scenesettings.update(get_time(scenesettings).time)) {
            set_scene(scenesettings.current_scene_next + 1);
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
          // create_new_mappings(i, next_instances, intermediates);

          // handle object movement (velocity added to position)
          update_object_positions(i, video);

          // handle collisions, gravity and "inherited" objects
          // update_object_interactions(i, next_instances, intermediates, instances, video);

          // convert javascript to renderable objects
          convert_objects_to_render_job(i, sc, video);

          // util::generator::copy_instances(i, intermediates, next_instances);
          scene_shapes_intermediate = scene_shapes_next;

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

      // util::generator::copy_instances(i, instances, next_instances);
      scene_shapes = scene_shapes_next;

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
  scene_shapes_intermediate = scene_shapes;
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

void native_generator::create_new_mappings(v8_interact& i,
                                           v8::Local<v8::Array>& next_instances,
                                           v8::Local<v8::Array>& intermediates) {
  next_instance_map.clear();
  intermediate_map.clear();
  for (size_t j = 0; j < next_instances->Length(); j++) {
    auto next = i.get_index(next_instances, j).As<v8::Object>();
    const auto unique_id = i.integer_number(next, "unique_id");
    next_instance_map[unique_id] = next;
  }
  for (size_t index = 0; index < intermediates->Length(); index++) {
    auto intermediate_instance = i.get_index(intermediates, index).As<v8::Object>();
    const auto uid = i.integer_number(intermediate_instance, "unique_id");
    intermediate_map[uid] = intermediate_instance;
  }
}

void native_generator::update_object_positions(v8_interact& i, v8::Local<v8::Object>& video) {
  // clear function caching
  cached_xy.clear();
  auto isolate = i.get_isolate();
  int64_t scenesettings_from_object_id = -1;
  int64_t scenesettings_from_object_id_level = -1;

  for (auto& shape : scene_shapes_next[scenesettings.current_scene_next]) {
    std::visit(overloaded{[&](data_staging::circle& shape) {
                            // auto unique_id = i.integer_number(instance, "unique_id");
                            // auto level = i.integer_number(instance, "level");
                            // std::string type = i.str(instance, "type");
                            // bool is_line = type == "line";
                            // bool is_script = type == "script";
                            //
                            // if (is_script) {
                            //   // TODO: this strategy does not support nested script objects
                            //   // TODO: we need to use stack for that
                            //   scenesettings_from_object_id = unique_id;
                            //   scenesettings_from_object_id_level = level;
                            // } else if (scenesettings_from_object_id_level == level) {

                            if (scenesettings_from_object_id_level == shape.level()) {
                              scenesettings_from_object_id = -1;
                              scenesettings_from_object_id_level = -1;
                            }

                            if (scenesettings_from_object_id == -1) {
                              // TODO:
                              //  update_time(i, instance, scenesettings);
                            } else {
                              // TODO:
                              // update_time(i, instance, scenesettings_objs[scenesettings_from_object_id]);
                            }

                            scalesettings.video_scale_next = i.double_number(video, "scale");

                            auto angle = shape.angle();
                            if (std::isnan(angle)) {
                              angle = 0.;
                            }
                            auto x = shape.position().x;
                            auto y = shape.position().y;
                            // auto x2 = i.double_number(instance, "x2");
                            // auto y2 = i.double_number(instance, "y2");
                            auto velocity = shape.velocity_speed();
                            auto vel_x = shape.velocity().x;
                            auto vel_y = shape.velocity().y;
                            // // auto vel_x2 = is_line ? i.double_number(instance, "vel_x2") : 0.0;
                            // // auto vel_y2 = is_line ? i.double_number(instance, "vel_y2") : 0.0;

                            velocity /= static_cast<double>(stepper.max_step);
                            x += (vel_x * velocity);
                            y += (vel_y * velocity);

                            // For now we only care about circles
                            // if (type == "circle" && i.double_number(instance, "radiussize") < 1000 /* todo create
                            // property of course */) {
                            if (shape.radius_size() < 1000 /* todo create property of course */) {
                              // TODO:
                              //  update_object_toroidal(i, instance, x, y);
                              const auto collision_group = shape.collision_group();
                              const auto gravity_group = shape.gravity_group();
                              if (!collision_group.empty()) {
                                qts.try_emplace(
                                    collision_group,
                                    quadtree(rectangle(position(-width() / 2, -height() / 2), width(), height()), 32));
                                auto x_copy = x;
                                auto y_copy = y;
                                // TODO: fix
                                //  fix_xy(i, instance, unique_id, x_copy, y_copy);
                                qts[collision_group].insert(point_type(position(x_copy, y_copy), shape.unique_id()));
                              }
                              if (!gravity_group.empty()) {
                                qts_gravity.try_emplace(
                                    gravity_group,
                                    quadtree(rectangle(position(-width() / 2, -height() / 2), width(), height()), 32));
                                qts_gravity[gravity_group].insert(point_type(position(x, y), shape.unique_id()));
                              }
                            }
                            shape.position_ref().x = x;
                            shape.position_ref().y = y;
                            // if (is_line) {
                            //   i.set_field(instance, "x2", v8::Number::New(isolate, x2));
                            //   i.set_field(instance, "y2", v8::Number::New(isolate, y2));
                            //                            }
                            // Needed?
                            // if (attempt == 1) {
                            //   i.set_field(instance, "steps", v8::Number::New(isolate, 1));
                            // }
                          },
                          [&](const data_staging::line& shape) {}},
               shape);
  }
}

void native_generator::update_object_toroidal(v8_interact& i, v8::Local<v8::Object>& instance, double& x, double& y) {
  auto toroidal = i.has_field(instance, "toroidal") ? i.str(instance, "toroidal") : "";
  if (!toroidal.empty()) {
    auto the_width = toroidals[toroidal].width;
    auto the_height = toroidals[toroidal].height;
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
    auto isolate = i.get_isolate();
    i.set_field(instance, "__warp_width__", v8::Number::New(isolate, the_width));
    i.set_field(instance, "__warp_height__", v8::Number::New(isolate, the_height));
    i.set_field(instance, "__warped_dist__", v8::Number::New(isolate, warped_dist));
  }
}

void native_generator::update_object_interactions(v8_interact& i,
                                                  v8::Local<v8::Array>& next_instances,
                                                  v8::Local<v8::Array>& intermediates,
                                                  v8::Local<v8::Array>& previous_instances,
                                                  v8::Local<v8::Object>& video) {
  stepper.reset_current();
  const auto isolate = i.get_isolate();

  // we cannot simply iterate over the next_instances array, since the array might mutate
  // during looping (since objects can trigger spawned objects, etc.) for this reason, create
  // a copy of the array and iterate over that

  std::vector<v8::Local<v8::Object>> instances;
  for (size_t index = 0; index < next_instances->Length(); index++) {
    auto next_instance = i.get_index(next_instances, index).As<v8::Object>();
    instances.push_back(next_instance);
  }

  for (auto& next_instance : instances) {
    auto instance_uid = i.integer_number(next_instance, "unique_id");
    auto find = intermediate_map.find(instance_uid);
    if (find == intermediate_map.end()) {
      continue;
    }
    auto intermediate_instance = find->second;

    auto motion_blur = !i.has_field(next_instance, "motion_blur") || i.boolean(next_instance, "motion_blur");

    if (!motion_blur) {
      i.set_field(next_instance, "steps", v8::Number::New(isolate, 1));
    } else {
      double dist = get_max_travel_of_object(i, next_instances, intermediate_instance, next_instance);
      if (dist > max_dist_found) {
        max_dist_found = dist;
      }
      auto steps = update_steps(dist);

      // TODO: why is this recorded_steps static
      // experimentally put this here for now
      static std::unordered_map<int64_t, int> recorded_steps;

      // below code is ugly and should be refactored soon anyway
      auto inherit = i.get(next_instance, "inherited");
      if (inherit->IsBoolean() && inherit.As<v8::Boolean>()->Value()) {
        // inherited, do not set our own dist etc.
        // EDIT: we need to read from cache as well, in case stuff got reverted..
        i.set_field(next_instance, "steps", v8::Number::New(isolate, recorded_steps[instance_uid]));
      } else {
        if (attempt == 1) {
          i.set_field(next_instance, "__dist__", v8::Number::New(isolate, dist));
          i.set_field(next_instance, "steps", v8::Number::New(isolate, steps));
          recorded_steps[instance_uid] = steps;
        } else if (attempt > 1) {
          i.set_field(next_instance, "steps", v8::Number::New(isolate, recorded_steps[instance_uid]));
        }
        // cascade to left and right props, temp
        if (i.has_field(next_instance, "props")) {
          auto p = i.get(next_instance, "props");
          if (p->IsObject()) {
            auto props = p.As<v8::Object>();
            for (const auto& left_or_right_str : {"left", "right"})
              if (i.has_field(props, left_or_right_str)) {
                auto l = i.get(props, left_or_right_str);
                if (l->IsArray()) {
                  auto a = l.As<v8::Array>();
                  for (size_t m = 0; m < a->Length(); m++) {
                    auto left_or_right = i.get_index(a, m).As<v8::Object>();
                    // TODO: lambdafy: below is copied from block above.
                    if (attempt == 1) {
                      auto use_dist = dist;
                      auto use_steps = steps;
                      if (i.has_field(left_or_right, "__dist__")) {
                        use_dist = std::max(use_dist, i.double_number(left_or_right, "__dist__"));
                      }
                      if (i.has_field(left_or_right, "steps")) {
                        use_steps = std::max(use_steps, (int)i.integer_number(left_or_right, "steps"));
                      }
                      auto a = i.integer_number(left_or_right, "unique_id");
                      recorded_steps[a] = use_steps;
                      i.set_field(left_or_right, "__dist__", v8::Number::New(isolate, dist));
                      i.set_field(left_or_right, "steps", v8::Number::New(isolate, use_steps));
                    } else if (attempt > 1) {
                      auto a = i.integer_number(left_or_right, "unique_id");
                      i.set_field(next_instance, "steps", v8::Number::New(isolate, recorded_steps[a]));
                    }
                    i.set_field(left_or_right, "inherited", v8::Boolean::New(isolate, true));
                  }
                } else if (l->IsObject()) {
                  auto left_or_right = l.As<v8::Object>();
                  if (attempt == 1) {
                    auto use_dist = dist;
                    auto use_steps = steps;
                    if (i.has_field(left_or_right, "__dist__")) {
                      use_dist = std::max(use_dist, i.double_number(left_or_right, "__dist__"));
                    }
                    if (i.has_field(left_or_right, "steps")) {
                      use_steps = std::max(use_steps, (int)i.integer_number(left_or_right, "steps"));
                    }
                    auto a = i.integer_number(left_or_right, "unique_id");
                    recorded_steps[a] = use_steps;
                    i.set_field(left_or_right, "__dist__", v8::Number::New(isolate, use_dist));
                    i.set_field(left_or_right, "steps", v8::Number::New(isolate, use_steps));
                  } else if (attempt > 1) {
                    auto a = i.integer_number(left_or_right, "unique_id");
                    i.set_field(next_instance, "steps", v8::Number::New(isolate, recorded_steps[a]));
                  }
                  i.set_field(left_or_right, "inherited", v8::Boolean::New(isolate, true));
                }
              }
          }
        }
      }
    }
    handle_collisions(i, next_instance, next_instances);
    handle_gravity(i, next_instance, next_instances);
  }
}

void native_generator::handle_collisions(v8_interact& i,
                                         v8::Local<v8::Object> instance,
                                         v8::Local<v8::Array> next_instances) {
  // Now do the collision detection part
  // NOTE: we multiple radius/size * 2.0 since we're not looking up a point, and querying the quadtree does
  // not check for overlap, but only whether the x,y is inside the specified range. If we don't want to miss
  // points on the edge of our circle, we need to widen the matching range.
  // TODO: for different sizes of circle collision detection, we need to somehow adjust the interface to this
  // lookup somehow.
  std::vector<point_type> found;
  auto type = i.str(instance, "type");
  auto collision_group = i.str(instance, "collision_group");
  if (collision_group == "undefined") {
    return;
  }
  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  auto unique_id = i.integer_number(instance, "unique_id");
  fix_xy(i, instance, unique_id, x, y);

  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");

  qts[collision_group].query(unique_id, circle(position(x, y), radius * 2.0, radiussize * 2.0), found);
  if (type == "circle" && i.double_number(instance, "radiussize") < 1000 /* todo create property of course */) {
    for (const auto& collide : found) {
      const auto unique_id2 = collide.userdata;
      auto instance2 = next_instance_map.at(unique_id2);
      handle_collision(i, instance, instance2);
    }
  }
}

void native_generator::handle_collision(v8_interact& i,
                                        v8::Local<v8::Object> instance,
                                        v8::Local<v8::Object> instance2) {
  const auto isolate = i.get_isolate();
  auto unique_id = i.integer_number(instance, "unique_id");
  auto unique_id2 = i.integer_number(instance2, "unique_id");
  auto last_collide = i.double_number(instance, "last_collide");

  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  fix_xy(i, instance, unique_id, x, y);

  auto x2 = i.double_number(instance2, "x");
  auto y2 = i.double_number(instance2, "y");
  fix_xy(i, instance2, unique_id2, x2, y2);

  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  auto radius2 = i.double_number(instance2, "radius");
  auto radiussize2 = i.double_number(instance2, "radiussize");
  auto mass = i.double_number(instance, "mass", 1.);
  auto mass2 = i.double_number(instance2, "mass", 1.);

  // If the quadtree reported a match, it doesn't mean the objects fully collide
  circle a(position(x, y), radius, radiussize);
  circle b(position(x2, y2), radius2, radiussize2);
  if (!a.overlaps(b)) return;

  if (!instance2->IsObject()) return;

  // they already collided, no need to let them collide again
  if (last_collide == unique_id2) return;

  // handle collision
  auto vel_x = i.double_number(instance, "vel_x");
  auto vel_y = i.double_number(instance, "vel_y");
  auto vel_x2 = i.double_number(instance2, "vel_x");
  auto vel_y2 = i.double_number(instance2, "vel_y");

  const auto normal = unit_vector(subtract_vector(vector2d(x, y), vector2d(x2, y2)));
  const auto ta = dot_product(vector2d(vel_x, vel_y), normal);
  const auto tb = dot_product(vector2d(vel_x2, vel_y2), normal);
  const auto optimized_p = (2.0 * (ta - tb)) / (mass + mass2);  // speed

  // save velocities
  const auto multiplied_vector = multiply_vector(normal, optimized_p);
  auto updated_vel1 = subtract_vector(vector2d(vel_x, vel_y), multiply_vector(multiplied_vector, mass2));
  i.set_field(instance, "vel_x", v8::Number::New(isolate, updated_vel1.x));
  i.set_field(instance, "vel_y", v8::Number::New(isolate, updated_vel1.y));
  auto updated_vel2 = add_vector(vector2d(vel_x2, vel_y2), multiply_vector(multiplied_vector, mass));
  i.set_field(instance2, "vel_x", v8::Number::New(isolate, updated_vel2.x));
  i.set_field(instance2, "vel_y", v8::Number::New(isolate, updated_vel2.y));

  // save collision
  i.set_field(instance, "last_collide", v8::Number::New(isolate, unique_id2));
  i.set_field(instance2, "last_collide", v8::Number::New(isolate, unique_id));

  // call 'on collision' event
  auto on = i.get(instance, "on").As<v8::Object>();
  i.call_fun(on, instance, "collide", instance2);
  auto on2 = i.get(instance2, "on").As<v8::Object>();
  i.call_fun(on2, instance2, "collide", instance);
}

void native_generator::handle_gravity(v8_interact& i,
                                      v8::Local<v8::Object> instance,
                                      v8::Local<v8::Array> next_instances) {
  std::vector<point_type> found;

  auto type = i.str(instance, "type");
  auto unique_id = i.integer_number(instance, "unique_id");
  auto gravity_group = i.str(instance, "gravity_group");
  if (gravity_group == "undefined") {
    return;
  }

  if (i.double_number(instance, "velocity", 0) == 0) return;  // skip this one.

  auto& video = genctx.video_obj;
  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  fix_xy(i, instance, unique_id, x, y);

  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  auto range = i.double_number(video, "gravity_range", 1000);

  qts_gravity[gravity_group].query(
      unique_id, circle(position(x, y), range + (radius * 2.0), range + (radiussize * 2.0)), found);

  if (type == "circle" && i.double_number(instance, "radiussize") < 1000 /* todo create property of course */) {
    vector2d acceleration(0, 0);
    for (const auto& in_range : found) {
      const auto unique_id2 = in_range.userdata;
      auto instance2 = next_instance_map.at(unique_id2);
      handle_gravity(i, instance, instance2, acceleration);
    }
    vector2d vel(i.double_number(instance, "vel_x", 0.), i.double_number(instance, "vel_y", 0.));
    vel = add_vector(vel, acceleration);
    i.set_field(instance, "vel_x", v8::Number::New(i.get_isolate(), vel.x));
    i.set_field(instance, "vel_y", v8::Number::New(i.get_isolate(), vel.y));
  }
}

void native_generator::handle_gravity(v8_interact& i,
                                      v8::Local<v8::Object> instance,
                                      v8::Local<v8::Object> instance2,
                                      vector2d& acceleration) {
  auto& video = genctx.video_obj;

  auto unique_id = i.integer_number(instance, "unique_id");
  auto x = i.double_number(instance, "x");
  auto y = i.double_number(instance, "y");
  fix_xy(i, instance, unique_id, x, y);

  auto unique_id2 = i.integer_number(instance2, "unique_id");
  auto x2 = i.double_number(instance2, "x");
  auto y2 = i.double_number(instance2, "y");
  fix_xy(i, instance2, unique_id2, x2, y2);

  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  auto radius2 = i.double_number(instance2, "radius");
  auto radiussize2 = i.double_number(instance2, "radiussize");
  auto mass = i.double_number(instance, "mass", 1.);
  auto mass2 = i.double_number(instance2, "mass", 1.);

  auto G = i.double_number(video, "gravity_G", 1);
  auto range = i.double_number(video, "gravity_range", 1000);

  // If the quadtree reported a match, it doesn't mean the objects fully collide
  circle a(position(x, y), radius + range, radiussize);
  circle b(position(x2, y2), radius2 + range, radiussize2);
  double dist = 0;
  if (!a.overlaps(b, dist)) return;
  if (!instance2->IsObject()) return;

  const auto constrain_dist_min = i.double_number(video, "gravity_constrain_dist_min", 5.);
  const auto constrain_dist_max = i.double_number(video, "gravity_constrain_dist_max", 25.);
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

void native_generator::update_time(v8_interact& i, v8::Local<v8::Object>& instance, scene_settings& scenesettings) {
  const auto time_settings = get_time(scenesettings);
  const auto execute = [&](double scene_time) {
    i.set_field(instance, "__time__", v8::Number::New(i.get_isolate(), scene_time));
    i.set_field(instance, "__global_time__", v8::Number::New(i.get_isolate(), time_settings.time));
    i.set_field(instance, "__elapsed__", v8::Number::New(i.get_isolate(), time_settings.elapsed));
    i.call_fun(
        instance, "time", scene_time, time_settings.elapsed, scenesettings.current_scene_next, time_settings.time);
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
double native_generator::get_max_travel_of_object(v8_interact& i,
                                                  v8::Local<v8::Array>& next_instances,
                                                  v8::Local<v8::Object>& previous_instance,
                                                  v8::Local<v8::Object>& instance) {
  // Update level for all objects
  // TODO: move to better place
  auto level = i.integer_number(instance, "level");
  auto type = i.str(instance, "type");
  auto is_line = type == "line";
  auto label = i.str(instance, "label");
  auto random_hash = i.str(instance, "__random_hash__");
  auto shape_scale = i.has_field(instance, "scale") ? i.double_number(instance, "scale") : 1.0;
  auto prev_shape_scale = i.has_field(previous_instance, "scale") ? i.double_number(previous_instance, "scale") : 1.0;

  const auto calculate = [this](v8_interact& i,
                                v8::Local<v8::Array>& next_instances,
                                v8::Local<v8::Object>& instance,
                                std::unordered_map<int64_t, v8::Local<v8::Object>>& lookup,
                                int64_t level,
                                bool is_line) {
    double angle = 0;

    data::coord pos;     // X, Y
    data::coord pos2;    // for lines there is an X2, Y2.
    data::coord parent;  // X, Y of parent
                         //    data::coord parent2; // X2, Y2 of parent
                         //    data::coord parent3; // centered X,Y of parent (i.o.w., middle of the line for lines)
    data::coord pos_for_parent;

    std::vector<v8::Local<v8::Object>> lineage;
    lineage.push_back(instance);
    auto parent_uid = i.integer_number(instance, "parent_uid");
    while (parent_uid != -1) {
      auto parent = lookup.at(parent_uid);
      lineage.push_back(parent);
      parent_uid = i.integer_number(parent, "parent_uid");
    }

    // reverse iterate over lineage vector
    for (auto it = lineage.rbegin(); it != lineage.rend(); ++it) {
      auto& current_obj = *it;
      const bool current_is_line = i.str(current_obj, "type") == "line";
      // const bool current_is_pivot =
      //    i.has_field(parent, "pivot") ? i.boolean(current_obj, "pivot") : false;

      // X,Y
      data::coord current{i.double_number(current_obj, "x"), i.double_number(current_obj, "y")};
      const double current_angle = i.double_number(current_obj, "angle", 0.);

      // X2, Y2, and centerX, centerY, for lines
      data::coord current2, current_center;
      if (current_is_line) {
        current2 = data::coord{i.double_number(current_obj, "x2"), i.double_number(current_obj, "y2")};
        current_center.x = ((current.x - current2.x) / 2) + current2.x;
        current_center.y = ((current.y - current2.y) / 2) + current2.y;
      }

      // OPTION 1: simply center from X,Y always (option for points and lines)
      // pos.add(current);

      // OPTION 2: center from X2, Y2 (only option for lines)
      // pos.add(current2);

      // OPTION 3: center from centerX, centerY (only option for lines)
      // pos.add(current_center);

      // for now, let's go for the most intuitive choice
      pos_for_parent.add(current_is_line ? current_center : current);
      pos.add(current);
      pos2.add(current_is_line ? current2 : current);

      // angle = current_is_pivot ? current_angle : (angle + current_angle);

      // total angle, cumulative no matter what
      if (!std::isnan(current_angle)) angle += current_angle;

      if (angle != 0.) {
        if (current_is_line) {
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
          pos_for_parent.x = parent.x + (cos(rads) * move);  // + current_center.x;
          pos_for_parent.y = parent.y + (sin(rads) * move);  // + current_center.y;
        } else {
          // current angle + angle with parent
          auto angle1 = angle + get_angle(parent.x, parent.y, pos.x, pos.y);
          while (angle1 > 360.) angle1 -= 360.;
          auto rads = angle1 * M_PI / 180.0;
          auto ratio = 1.0;
          auto dist = get_distance(0, 0, current.x, current.y);
          auto move = dist * ratio * -1;
          pos.x = parent.x + (cos(rads) * move);
          pos.y = parent.y + (sin(rads) * move);
          pos_for_parent.x = parent.x + (cos(rads) * move);
          pos_for_parent.y = parent.y + (sin(rads) * move);
        }
      }

      // now we can update the parent for the next level we're about to handle
      parent = pos_for_parent;
    }

    // store transitive x & y etc.
    i.set_field(instance, "transitive_x", v8::Number::New(i.get_isolate(), pos.x));
    i.set_field(instance, "transitive_y", v8::Number::New(i.get_isolate(), pos.y));
    if (is_line) {
      i.set_field(instance, "transitive_x2", v8::Number::New(i.get_isolate(), pos2.x));
      i.set_field(instance, "transitive_y2", v8::Number::New(i.get_isolate(), pos2.y));
    }

    // pass along x, y, x2, y2.
    if (i.has_field(instance, "props")) {
      const auto process_obj =
          [&i, &pos, this](v8::Local<v8::Object>& o, const std::string& inherit_x, const std::string& inherit_y) {
            const auto unique_id = i.integer_number(o, "unique_id");
            const auto find = next_instance_map.find(unique_id);
            if (find != next_instance_map.end()) {
              auto other_val = find->second;
              if (other_val->IsObject()) {
                auto other = other_val.As<v8::Object>();
                i.set_field(other, inherit_x, v8::Number::New(i.get_isolate(), pos.x));
                i.set_field(other, inherit_y, v8::Number::New(i.get_isolate(), pos.y));
              }
            }
          };
      const auto process = [&i, &process_obj](const v8::Local<v8::Value>& field_value,
                                              const std::string& inherit_x,
                                              const std::string& inherit_y) {
        if (field_value->IsArray()) {
          auto a = field_value.As<v8::Array>();
          for (size_t l = 0; l < a->Length(); l++) {
            auto o = i.get_index(a, l).As<v8::Object>();
            process_obj(o, inherit_x, inherit_y);
          }
        } else if (field_value->IsObject()) {
          auto o = field_value.As<v8::Object>();
          process_obj(o, inherit_x, inherit_y);
        }
      };
      auto props = i.v8_obj(instance, "props");
      auto obj_fields = i.prop_names(props);
      for (size_t k = 0; k < obj_fields->Length(); k++) {
        auto field_name = i.get_index(obj_fields, k);
        auto field_value = i.get(props, field_name);
        if (!field_value->IsObject()) continue;
        auto str = i.str(obj_fields, k);
        if (str == "left") {
          process(field_value, "inherited_x", "inherited_y");
        } else if (str == "right") {
          process(field_value, "inherited_x2", "inherited_y2");
        }
      }
    }

    return std::make_tuple(pos.x, pos.y, pos2.x, pos2.y);
  };

  auto [x, y, x2, y2] = calculate(i, next_instances, instance, next_instance_map, level, is_line);
  auto [prev_x, prev_y, prev_x2, prev_y2] =
      calculate(i, next_instances, previous_instance, intermediate_map, level, is_line);

  // Calculate how many pixels are maximally covered by this instance, this is currently very simplified
  x *= scalesettings.video_scale_next * shape_scale;
  prev_x *= scalesettings.video_scale_intermediate * prev_shape_scale;
  y *= scalesettings.video_scale_next * shape_scale;
  prev_y *= scalesettings.video_scale_intermediate * prev_shape_scale;

  //  // TODO: make smarter for circles and lines, this is just temporary code
  //  rectangle canvas(position(-canvas_w / 2., -canvas_h/2.), position(canvas_w / 2., canvas_h/2.));
  //  if (is_line) {
  //    rectangle shape_rect(position(x, y), position(x2, y2));
  //    rectangle prev_shape_rect(position(prev_x - rad, prev_y - rad), position(prev_x + rad, prev_y + rad));
  //    if (!canvas.overlaps(shape_rect) && !canvas.overlaps(prev_shape_rect)) {
  //      return 0.;
  //    }
  //  } else {
  //    rectangle shape_rect(position(x - rad, y - rad), position(x + rad, y + rad));
  //    rectangle prev_shape_rect(position(prev_x - rad, prev_y - rad), position(prev_x + rad, prev_y + rad));
  //    if (!canvas.overlaps(shape_rect) && !canvas.overlaps(prev_shape_rect)) {
  //      return 0.;
  //    }
  //  }
  auto dist = sqrt(pow(x - prev_x, 2) + pow(y - prev_y, 2));
  // x2, y2
  if (is_line) {
    x2 *= scalesettings.video_scale_next * shape_scale;
    prev_x2 *= scalesettings.video_scale_intermediate * prev_shape_scale;
    y2 *= scalesettings.video_scale_next * shape_scale;
    prev_y2 *= scalesettings.video_scale_intermediate * prev_shape_scale;
    dist = std::max(dist, sqrt(pow(x2 - prev_x2, 2) + pow(y2 - prev_y2, 2)));
  }

  // radius
  auto radius = i.double_number(instance, "radius");
  auto radiussize = i.double_number(instance, "radiussize");
  auto prev_radius = i.double_number(previous_instance, "radius");
  auto prev_radiussize = i.double_number(previous_instance, "radiussize");
  auto rad = radius + radiussize;
  auto prev_rad = prev_radius + prev_radiussize;
  rad *= scalesettings.video_scale_next * shape_scale;
  prev_rad *= scalesettings.video_scale_intermediate * prev_shape_scale;
  // TODO: this is not 100% accurate, if an object is moving and expanding its radius the max distance can be more
  // but this should at least result in a decent enough effect in most cases
  dist = std::max(dist, fabs(prev_rad - rad));

  // Make sure that we do not include any warped distance
  if (i.has_field(instance, "__warped_dist__")) {
    dist -= i.double_number(instance, "__warped_dist__");
    // If object moves two pixels to the right, is then warped 500 px to the left
    // It has traveled 498, and the above statement has resulted in a value of -2.
    // If it moves to the left for 2 pixels, and is moved 500px to the right, the
    // distance is 502px - 500px = 2px as well.
    dist = fabs(dist);
  }

  // TODO: proper support for connected objects, so they can inherit the warped stuff!
  if (dist > 100) {
    // logger(INFO) << "Warning, distance found > 100, id hash: " << random_hash << std::endl;
  }
  return dist;
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
  // Update level for all objects
  std::visit(overloaded{
                 [&](data_staging::circle& shape) {
                   auto level = 0;      // shape.level;
                   auto exists = true;  // !i.has_field(instance, "exists") || i.boolean(instance, "exists");
                   if (!exists) return;
                   // auto type = i.str(instance, "type");
                   // auto is_line = type == "line";
                   // parents_stack[level] = instance;

                   // See if we require this step for this object
                   // auto steps = i.integer_number(instance, "steps");
                   // if (minimize_steps_per_object && !sc.do_step(steps, stepper.next_step)) {
                   // TODO: make this a property also for objects, if they are vibrating they need this
                   //  return;
                   //}
                   // auto id = i.str(instance, "id");
                   // auto label = i.str(instance, "label");
                   // auto time = i.double_number(instance, "__time__");
                   // auto transitive_x = i.double_number(instance, "transitive_x");
                   // auto transitive_y = i.double_number(instance, "transitive_y");
                   // auto transitive_x2 = is_line ? i.double_number(instance, "transitive_x2") : 0.0;
                   // auto transitive_y2 = is_line ? i.double_number(instance, "transitive_y2") : 0.0;
                   // auto vel_x = i.double_number(instance, "vel_x");
                   // auto vel_y = i.double_number(instance, "vel_y");

                   // auto inherited_x = i.has_field(instance, "inherited_x") ?
                   // i.double_number(instance, "inherited_x") : 0.; auto inherited_y =
                   // i.has_field(instance, "inherited_y") ? i.double_number(instance, "inherited_y")
                   // : 0.; auto inherited_x2 = i.has_field(instance, "inherited_x2") ?
                   // i.double_number(instance, "inherited_x2") : 0.; auto inherited_y2 =
                   // i.has_field(instance, "inherited_y2") ? i.double_number(instance,
                   // "inherited_y2") : 0.;
                   //
                   // if (inherited_x) transitive_x = inherited_x;
                   // if (inherited_y) transitive_y = inherited_y;
                   // if (inherited_x2) transitive_x2 = inherited_x2;
                   // if (inherited_y2) transitive_y2 = inherited_y2;

                   auto radius = shape.radius();           // i.double_number(instance, "radius");
                   auto radiussize = shape.radius_size();  // i.double_number(instance, "radiussize");
                   // auto seed = i.double_number(instance, "seed");
                   auto blending_type = shape.blending_type();  // i.has_field(instance, "blending_type") ?
                                                                // i.integer_number(instance, "blending_type")
                                                                //              : data::blending_type::normal;
                   auto scale = shape.scale();  // i.has_field(instance, "scale") ? i.double_number(instance, "scale")
                                                // : 1.0; auto unique_id = i.integer_number(instance, "unique_id");

    // auto shape_opacity = i.double_number(instance, "opacity");
    // auto motion_blur = i.boolean(instance, "motion_blur");
    // auto warp_width = i.has_field(instance, "__warp_width__") ? i.integer_number(instance, "__warp_width__") : 0;
    // auto warp_height = i.has_field(instance, "__warp_height__") ? i.integer_number(instance, "__warp_height__") : 0;

    // auto text = i.str(instance, "text");
    // auto text_align = i.str(instance, "text_align");
    // auto text_size = i.integer_number(instance, "text_size");
    // auto text_fixed = i.boolean(instance, "text_fixed");
    // auto text_font = i.has_field(instance, "text_font") ? i.str(instance, "text_font") : "";

    // TODO: might not need this param after all
// auto dist = i.double_number(instance, "__dist__");
#define DEBUG_NUM_SHAPES
#ifdef DEBUG_NUM_SHAPES
// auto random_hash = i.str(instance, "__random_hash__");
#endif

                   // temp
                   double transitive_x = shape.position().x;
                   double transitive_y = shape.position().y;

                   data::shape new_shape;
                   // new_shape.time = time;
                   new_shape.x = transitive_x;
                   new_shape.y = transitive_y;
                   new_shape.level = level;
                   // new_shape.dist = dist;

                   new_shape.gradients_.clear();
                   new_shape.textures.clear();
                   std::string gradient_id_str;
                   // util::generator::copy_gradient_from_object_to_shape(i, instance, new_shape, gradients,
                   // &gradient_id_str); util::generator::copy_texture_from_object_to_shape(i, instance, new_shape,
                   // textures);

                   // temp hack
                   std::string namespace_ = "";                 // i.str(source_object, "namespace", "");
                   std::string gradient_id = shape.gradient();  // namespace_ + i.str(source_object, "gradient");

                   if (!gradient_id.empty()) {
                     // if (gradient_id_str) {
                     //   *gradient_id_str += gradient_id;
                     // }
                     if (new_shape.gradients_.empty()) {
                       auto& known_gradients_map = gradients;
                       if (known_gradients_map.find(gradient_id) != known_gradients_map.end()) {
                         new_shape.gradients_.emplace_back(1.0, known_gradients_map[gradient_id]);
                       }
                     }
                   }
                   // auto gradient_array = i.v8_array(source_object, "gradients");
                   // if (destination_shape.gradients_.empty()) {
                   //   for (size_t k = 0; k < gradient_array->Length(); k++) {
                   //     auto gradient_data = i.get_index(gradient_array, k).As<v8::Array>();
                   //     if (!gradient_data->IsArray()) {
                   //       continue;
                   //     }
                   //     auto opacity = i.double_number(gradient_data, size_t(0));
                   //     auto gradient_id = namespace_ + i.str(gradient_data, size_t(1));
                   //     if (gradient_id_str) {
                   //       if (!gradient_id_str->empty()) *gradient_id_str += ",";
                   //       *gradient_id_str += gradient_id;
                   //     }
                   //     destination_shape.gradients_.emplace_back(opacity,
                   //     known_gradients_map[gradient_id]);
                   //   }
                   // }

                   // while (level > 0) {
                   //   level--;
                   //   util::generator::copy_gradient_from_object_to_shape(
                   //       i, parents_stack[level], new_shape, gradients, &gradient_id_str);
                   //   util::generator::copy_texture_from_object_to_shape(i, parents_stack[level], new_shape,
                   //   textures); auto s = i.double_number(parents_stack[level], "scale"); if (!std::isnan(s)) {
                   //     scale *= s;
                   //   }
                   // }
                   // new_shape.gradient_id_str = gradient_id_str;
                   if (new_shape.gradients_.empty()) {
                     new_shape.gradients_.emplace_back(1.0, data::gradient{});
                     new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
                     new_shape.gradients_[0].second.colors.emplace_back(0.0, data::color{1.0, 1, 1, 1});
                     new_shape.gradients_[0].second.colors.emplace_back(1.0, data::color{0.0, 0, 0, 1});
                   }
                   new_shape.z = 0;
                   // new_shape.vel_x = vel_x;
                   // new_shape.vel_y = vel_y;
                   new_shape.radius = radius;
                   new_shape.radius_size = radiussize;
                   new_shape.blending_ = blending_type;
                   new_shape.scale = scale;
                   new_shape.opacity = 1.0;  // std::isnan(shape_opacity) ? 1.0 : shape_opacity;
                                             // new_shape.unique_id = unique_id;
#ifdef DEBUG_NUM_SHAPES
    // new_shape.random_hash = random_hash;
#endif
                   new_shape.seed = seed;
                   // new_shape.id = id;
                   // new_shape.label = label;
                   // new_shape.motion_blur = motion_blur;
                   // new_shape.warp_width = warp_width;
                   // new_shape.warp_height = warp_height;

                   // if (type == "circle") {
                   new_shape.type = data::shape_type::circle;
                   // } else if (type == "line") {
                   //   new_shape.type = data::shape_type::line;
                   //   new_shape.x2 = transitive_x2;
                   //   new_shape.y2 = transitive_y2;
                   // } else if (type == "text") {
                   //   new_shape.type = data::shape_type::text;
                   //   new_shape.text = text;
                   //   new_shape.text_size = text_size;
                   //   new_shape.align = text_align;
                   //   new_shape.text_fixed = text_fixed;
                   //   new_shape.text_font = text_font;
                   // } else if (type == "script") {
                   //   new_shape.type = data::shape_type::script;
                   // } else {
                   //   new_shape.type = data::shape_type::none;
                   // }
                   // wrap this in a proper add method
                   if (stepper.next_step != stepper.max_step) {
                     indexes[shape.unique_id()][stepper.current_step] = job->shapes[stepper.current_step].size();
                   } else {
                     new_shape.indexes = indexes[shape.unique_id()];
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
                 },
                 [](data_staging::line& shape) {
                   // TODO
                 },
             },
             shape);
}

std::shared_ptr<data::job> native_generator::get_job() const {
  return job;
}

v8::Local<v8::Object> native_generator::spawn_object_native(v8::Local<v8::Object> spawner, v8::Local<v8::Object> obj) {
  auto& i = genctx.i();
  auto created_instance =
      util::generator::instantiate_object_from_scene(i, genctx.objects, genctx.instances_next, obj, &spawner);
  next_instance_map[i.integer_number(created_instance, "unique_id")] = created_instance;
  create_bookkeeping_for_script_objects(created_instance);
  return created_instance;
}

// TODO: will refactor soon
void native_generator::fix_xy(v8_interact& i, v8::Local<v8::Object>& instance, int64_t uid, double& x, double& y) {
  // experimental function caching
  double xx = 0;
  double yy = 0;
  auto find = cached_xy.find(uid);
  if (find != cached_xy.end()) {
    std::tie(xx, yy) = find->second;
    x += xx;
    y += yy;
    return;
  }
  auto parent_uid = i.integer_number(instance, "parent_uid", -1);
  while (parent_uid != -1) {
    auto parent = next_instance_map.at(parent_uid);
    if (i.str(parent, "type", "") == "script") {
      xx += i.double_number(parent, "x");
      yy += i.double_number(parent, "y");
    }
    parent_uid = i.integer_number(parent, "parent_uid");
  }
  cached_xy[uid] = std::make_pair(xx, yy);
  x += xx;
  y += yy;
}