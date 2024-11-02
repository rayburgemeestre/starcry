/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scenes.h"

#include "generator.h"
#include "job_holder.h"
#include "util/generator_context.h"

// #define DEBUG2

namespace interpreter {

scenes::scenes(generator& gen,
               std::shared_ptr<v8_wrapper>& context,
               std::shared_ptr<generator_context>& genctx,
               instantiator& instantiator,
               frame_stepper& stepper,
               job_holder& holder)
    : gen_(gen), context(context), genctx(genctx), instantiator_(instantiator), stepper(stepper), job_holder_(holder) {}

scenes scenes::clone() {
  return scenes{*this};
}

void scenes::load_from(const scenes& other) {
  this->scene_shapes = other.scene_shapes;
  this->scene_shapes_next = other.scene_shapes_next;
  this->instantiated_objects = other.instantiated_objects;
  this->scene_shapes_intermediate = other.scene_shapes_intermediate;
  this->scenesettings = other.scenesettings;
  this->scenesettings_objs = other.scenesettings_objs;
}

void scenes::initialize() {
  scenesettings.scene_initialized = std::numeric_limits<size_t>::max();
  scenesettings.scene_initialized_previous = std::numeric_limits<size_t>::max();

  refresh_scenesettings();
}

void scenes::add_scene() {
  scene_shapes.emplace_back();
  scene_shapes_next.emplace_back();
  scene_shapes_intermediate.emplace_back();
  instantiated_objects.emplace_back();
}

void scenes::refresh_scenesettings() {
  // refresh the scene object to get rid of left-over state
  scene_settings tmp;
  std::swap(scenesettings, tmp);

  // throw away all the scene information for script objects
  scenesettings_objs.clear();

  // throw away any existing instances from array
  for (auto& scene_shapes_vec : scene_shapes_next) scene_shapes_vec.clear();
}

void scenes::set_scene(size_t scene) {
  if (scenesettings.current_scene_next == std::numeric_limits<size_t>::max())
    scenesettings.current_scene_next = scene;
  else
    scenesettings.current_scene_next = std::max(scenesettings.current_scene_next, scene);
  if (scenesettings.scene_initialized == std::numeric_limits<size_t>::max() ||
      scenesettings.current_scene_next > scenesettings.scene_initialized) {
    scenesettings.scene_initialized_previous = scenesettings.scene_initialized;  // in case we need to revert
    scenesettings.scene_initialized = scenesettings.current_scene_next;
    create_object_instances();
  }
#ifdef DEBUG2
  logger(DEBUG) << "##[ scenes::set_scene() ]##" << std::endl;
  gen_.debug_print_all();
#endif
}

void scenes::switch_scene() {
  // whenever we switch to a new scene, we'll copy all the object state from the previous scene
  if (scenesettings.current_scene_next > 0) {
    logger(INFO) << "Switching to new scene, copying all state from previous." << std::endl;
    scene_shapes[scenesettings.current_scene_next] = scene_shapes[scenesettings.current_scene_next - 1];
    scene_shapes_next[scenesettings.current_scene_next] = scene_shapes_next[scenesettings.current_scene_next - 1];
    scene_shapes_intermediate[scenesettings.current_scene_next] =
        scene_shapes_intermediate[scenesettings.current_scene_next - 1];
  } else {
    logger(DEBUG) << "Stay in existing scene " << scenesettings.current_scene_next << ", not copying." << std::endl;
  }
#ifdef DEBUG2
  logger(DEBUG) << "##[ scenes::switch_scene() ]##" << std::endl;
  gen_.debug_print_all();
#endif
}

void scenes::append_instantiated_objects() {
  scene_shapes_next[scenesettings.current_scene_next].insert(
      std::end(scene_shapes_next[scenesettings.current_scene_next]),
      std::begin(instantiated_objects[scenesettings.current_scene_next]),
      std::end(instantiated_objects[scenesettings.current_scene_next]));
  instantiated_objects[scenesettings.current_scene_next].clear();
}

void scenes::prepare_scene() {
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
      _set_scene_sub_object(settings, settings.current_scene_next + 1);
    }
  }
}

/* inline */ interpreter::time_settings scenes::get_time(scene_settings& scenesettings) const {
  auto& stepper = this->stepper;
  auto& job = job_holder_.get_ref();
  // TODO: move state, config, options etc. into some kind of aggregate
  auto& max_frames = gen_.state().max_frames;
  auto& use_fps = gen_.config().fps;
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
    const auto perc_of_t = desired_duration / scenesettings.scenes_duration;  // e.g. 0.33
    const auto Tstart = scenesettings.parent_offset;                          // e.g. 0.50
    const auto Tend = scenesettings.parent_offset + perc_of_t;                // e.g. 0.83
    const auto T = (t - Tstart) / (Tend - Tstart);                            // e.g. 0.91
    const auto E = e;
    auto S = std::clamp((T - scenesettings.offset_next) / next_scene_duration, 0., 1.);
    return time_settings{T, E, S};
  }

  // block for normal types
  const auto scene_time = std::clamp((t - scenesettings.offset_next) / next_scene_duration, 0., 1.);
  return time_settings{t, e, scene_time};
}

void scenes::update() {
  scenesettings.update();
  for (auto& iter : scenesettings_objs) {
    iter.second.update();
  }
}

void scenes::commit() {
  scenesettings.commit();
  for (auto& [_, scenesetting] : scenesettings_objs) {
    scenesetting.commit();
  }
}

void scenes::revert() {
  scenesettings.revert();
  for (auto& iter : scenesettings_objs) {
    iter.second.revert();
  }
}

void scenes::reset() {
  scene_shapes.clear();
  scene_shapes_next.clear();
  scene_shapes_intermediate.clear();
  instantiated_objects.clear();
}

void scenes::commit_scene_shapes() {
#ifdef DEBUG2
  logger(DEBUG) << "##[ scenes::commit_scene_shapes(BEFORE) ]##" << std::endl;
  gen_.debug_print_all();
#endif
  scene_shapes = scene_shapes_next;
#ifdef DEBUG2
  logger(DEBUG) << "##[ scenes::commit_scene_shapes(AFTER) ]##" << std::endl;
  gen_.debug_print_all();
#endif
}
void scenes::commit_scene_shapes_intermediates() {
#ifdef DEBUG2
  logger(DEBUG) << "##[ scenes::commit_scene_shapes_intermediates(BEFORE) ]##" << std::endl;
  gen_.debug_print_all();
#endif
  scene_shapes_intermediate = scene_shapes_next;
#ifdef DEBUG2
  logger(DEBUG) << "##[ scenes::commit_scene_shapes_intermediates(AFTER) ]##" << std::endl;
  gen_.debug_print_all();
#endif
}
void scenes::reset_scene_shapes_next() {
  scene_shapes_next = scene_shapes;
#ifdef DEBUG2
  logger(DEBUG) << "##[ scenes::reset_scene_shapes_next() ]##" << std::endl;
  gen_.debug_print_all();
#endif
}

void scenes::dump() {
  const auto details = [](const auto& vecvec) {
    size_t index = 0;
    for (const auto& v : vecvec) {
      logger(DEBUG) << "scenes::dump::details - " << index << " - " << v.size() << std::endl;
      index++;
    }
  };
  logger(DEBUG) << "scenes::dump - scene_shapes.size() = " << this->scene_shapes.size() << std::endl;
  details(this->scene_shapes);
  logger(DEBUG) << "scenes::dump - scene_shapes_next.size() = " << this->scene_shapes_next.size() << std::endl;
  details(this->scene_shapes_next);
  logger(DEBUG) << "scenes::dump - scene_shapes_intermediate.size() = " << this->scene_shapes_intermediate.size()
                << std::endl;
  details(this->scene_shapes_intermediate);
}

void scenes::memory_dump() {
  const auto size_in_megabytes = [](auto& f) {
    size_t t = 0;
    for (const auto& x : f) {
      t += (x.size() * sizeof(data_staging::shape_t));
    }
    return t / 1024. / 1024.;
  };
  logger(DEBUG) << "scenes::memory_dump:" << std::endl;
  logger(DEBUG) << "- scene_shapes.size() = " << size_in_megabytes(this->scene_shapes) << " MB." << std::endl;
  logger(DEBUG) << "- scene_shapes_next.size() = " << size_in_megabytes(this->scene_shapes_next) << " MB." << std::endl;
  logger(DEBUG) << "- scene_shapes_intermediate.size() = " << size_in_megabytes(this->scene_shapes_intermediate)
                << " MB." << std::endl;
}

void scenes::reset_scene_shapes_intermediates() {
  scene_shapes_intermediate = scene_shapes;
#ifdef DEBUG2
  logger(DEBUG) << "##[ scenes::reset_scene_shapes_intermediates() ]##" << std::endl;
  gen_.debug_print_all();
#endif
}

bool scenes::cleanup_destroyed_objects() {
  size_t total_destroyed = 0;
  for (auto& scenes : scene_shapes_next) {
    scenes.erase(std::remove_if(scenes.begin(),
                                scenes.end(),
                                [&scenes, &total_destroyed](auto& shape) {
                                  bool ret = false;
                                  size_t destroyed = 0;
                                  meta_callback(shape, [&]<typename T>(T& shape) {
                                    ret = shape.meta_cref().is_destroyed();
                                    if (ret) {
                                      destroyed++;
                                      // we should update levels for this about to be erased parent
                                      // we can do this here because this does not involve removing
                                      // elements, etc., so we won't invalidate any iterators

                                      // for now we'll just orphan all the objects that are affected
                                      // let's check only one level for now

                                      // TODO: we should do this recursively..., but this is a nice start
                                      for (auto& scene_shape : scenes) {
                                        meta_callback(scene_shape, [&]<typename T2>(T2& shape2) {
                                          if (shape2.meta_cref().parent_uid() == shape.meta_cref().unique_id()) {
                                            shape2.meta_ref().set_level(0);
                                            shape2.meta_ref().set_parent_uid(-1);
                                          }
                                        });
                                      }
                                    }
                                  });
                                  total_destroyed += destroyed;
                                  return ret;
                                }),
                 scenes.end());
  }
  return total_destroyed > 0;
}

std::vector<data_staging::shape_t>& scenes::shapes_current_scene() {
  return scene_shapes[scenesettings.current_scene_next];
}

std::vector<data_staging::shape_t>& scenes::next_shapes_current_scene() {
  return scene_shapes_next[scenesettings.current_scene_next];
}

std::vector<data_staging::shape_t>& scenes::intermediate_shapes_current_scene() {
  return scene_shapes_intermediate[scenesettings.current_scene_next];
}

std::vector<data_staging::shape_t>& scenes::instantiated_objects_current_scene() {
  return instantiated_objects[scenesettings.current_scene_next];
}

double scenes::get_duration(int64_t unique_id) {
  return scenesettings_objs[unique_id].scenes_duration;
}

void scenes::set_duration(int64_t unique_id, double duration) {
  logger(DEBUG) << "scene set duration for " << unique_id << " to " << duration << std::endl;
  scenesettings_objs[unique_id].scenes_duration = duration;
}

void scenes::set_durations(int64_t unique_id, std::vector<double>& durations) {
  // create comma-separated string with all durations
  std::stringstream ss;
  for (auto& duration : durations) {
    ss << duration << ",";
  }
  logger(DEBUG) << "scene set durations for " << unique_id << " to " << ss.str() << std::endl;
  scenesettings_objs[unique_id].scene_durations = durations;
}

void scenes::set_desired_duration(int64_t unique_id, double value) {
  logger(DEBUG) << "scene set desired duration for " << unique_id << " to " << value << std::endl;
  scenesettings_objs[unique_id].desired_duration = value;
}

size_t scenes::current() {
  return scenesettings.current_scene_next;
}

void scenes::set_scene_sub_object(int64_t unique_id) {
  _set_scene_sub_object(scenesettings_objs[unique_id], 0);
}

void scenes::_set_scene_sub_object(scene_settings& scenesettings, size_t scene) {
  if (scenesettings.current_scene_next == std::numeric_limits<size_t>::max())
    scenesettings.current_scene_next = scene;
  else
    scenesettings.current_scene_next = std::max(scenesettings.current_scene_next, scene);
  if (scenesettings.scene_initialized == std::numeric_limits<size_t>::max() ||
      scenesettings.current_scene_next > scenesettings.scene_initialized) {
    scenesettings.scene_initialized = scenesettings.current_scene_next;
    // TODO: implement:
    //  create_object_instances();
  }
}

void scenes::create_object_instances() {
  // This function is called whenever a scene is set. (once per scene)
  context->enter_object("script", [&](v8::Isolate* isolate, v8::Local<v8::Value> val) {
    // enter_objects creates a new isolate, using the old gives issues, so we'll recreate
    genctx = std::make_shared<generator_context>(val, current());
    genctx->set_scene(current());

    switch_scene();

    instantiator_.instantiate_additional_objects_from_new_scene(genctx->scene_objects, 0);

    // since this is invoked directly after a scene change, and in the very beginning, make sure this state is part of
    // the instances "current" frame, or reverting (e.g., due to motion blur requirements) will discard all of this.
    commit_scene_shapes();
  });
}

}  // namespace interpreter
