/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "interactor.h"
#include "abort_exception.hpp"
#include "generator.h"
#include "object_definitions.h"

namespace interpreter {

interactor::interactor(generator& gen, toroidal_manager& tm, object_definitions& definitions)
    : gen_(gen), toroidal_manager_(tm), definitions_(definitions) {}

void interactor::reset() {
  qts.clear();
  qts_gravity.clear();
  unique_groups.clear();
}

void interactor::update_interactions() {
  const auto handle_pass1 = [&]<typename T>(data_staging::shape_t& abstract_shape, T& shape, data_staging::meta& meta) {
    if constexpr (std::is_same_v<T, data_staging::circle> || std::is_same_v<T, data_staging::script>) {
      double x = shape.transitive_location_cref().position_cref().x;
      double y = shape.transitive_location_cref().position_cref().y;
      double diff_x = 0;
      double diff_y = 0;
      update_object_toroidal(shape.toroidal_ref(), x, y, diff_x, diff_y);
      shape.transitive_location_ref().position_ref().x = x;
      shape.transitive_location_ref().position_ref().y = y;
      shape.location_ref().position_ref().x += diff_x;
      shape.location_ref().position_ref().y += diff_y;
      const auto collision_group = shape.behavior_cref().collision_group();
      const auto gravity_group = shape.behavior_cref().gravity_group();
      const auto unique_group = shape.behavior_cref().unique_group();

      if (!collision_group.empty()) {
        qts.try_emplace(collision_group,
                        quadtree(rectangle(position(-gen_.state().canvas_w / 2, -gen_.state().canvas_h / 2),
                                           gen_.state().canvas_w,
                                           gen_.state().canvas_h),
                                 32));
        qts[collision_group].insert(point_type(position(x, y), shape.meta_cref().unique_id()));
      }
      if (!gravity_group.empty()) {
        qts_gravity.try_emplace(gravity_group,
                                quadtree(rectangle(position(-gen_.state().canvas_w / 2, -gen_.state().canvas_h / 2),
                                                   gen_.state().canvas_w,
                                                   gen_.state().canvas_h),
                                         32));
        qts_gravity[gravity_group].insert(point_type(position(x, y), shape.meta_cref().unique_id()));
      }
      if (!unique_group.empty()) {
        unique_groups[unique_group].add(x, y);
      }
    }
    if constexpr (std::is_same_v<T, data_staging::line>) {
      double x = shape.transitive_line_start_ref().position_ref().x;
      double y = shape.transitive_line_start_ref().position_ref().y;
      double x2 = shape.transitive_line_end_ref().position_ref().x;
      double y2 = shape.transitive_line_end_ref().position_ref().y;
      const auto unique_group = shape.behavior_cref().unique_group();
      if (!unique_group.empty()) {
        unique_groups[unique_group].add(x, y, x2, y2);
      }
    }
  };

  const auto handle_pass2 = [&]<typename T>(data_staging::shape_t& abstract_shape, T& shape, data_staging::meta& meta) {
    handle_collisions(abstract_shape);
    handle_gravity(abstract_shape);
  };

  // first pass transitive xy become set
  for (auto& abstract_shape : gen_.scenes_.next_shapes_current_scene()) {
    std::visit(overloaded{[](std::monostate) {},
                          [&](data_staging::circle& c) {
                            handle_pass1(abstract_shape, c, c.meta_ref());
                          },
                          [&](data_staging::ellipse& e) {
                            handle_pass1(abstract_shape, e, e.meta_ref());
                          },
                          [&](data_staging::line& l) {
                            handle_pass1(abstract_shape, l, l.meta_ref());
                          },
                          [&](data_staging::text& t) {
                            handle_pass1(abstract_shape, t, t.meta_ref());
                          },
                          [&](data_staging::script& s) {
                            handle_pass1(abstract_shape, s, s.meta_ref());
                          }},
               abstract_shape);
  }

  // second pass depends on knowing transitive xy
  for (auto& abstract_shape : gen_.scenes_.next_shapes_current_scene()) {
    std::visit(overloaded{[](std::monostate) {},
                          [&](data_staging::circle& c) {
                            handle_pass2(abstract_shape, c, c.meta_ref());
                            for (const auto& cascade_out : c.cascade_out_cref()) {
                              auto& other = gen_.object_lookup_.at(cascade_out.unique_id()).get();
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
                          [&](data_staging::ellipse& e) {
                            // TODO: duplicated code from above with ellipse -> we need to re-use
                            handle_pass2(abstract_shape, e, e.meta_ref());
                            for (const auto& cascade_out : e.cascade_out_cref()) {
                              auto& other = gen_.object_lookup_.at(cascade_out.unique_id()).get();
                              if (auto other_line = std::get_if<data_staging::line>(&other)) {
                                if (cascade_out.type() == cascade_type::start) {
                                  other_line->line_start_ref().position_ref().x = e.location_ref().position_cref().x;
                                  other_line->line_start_ref().position_ref().y = e.location_ref().position_cref().y;
                                  other_line->transitive_line_start_ref().position_ref().x =
                                      e.transitive_location_ref().position_cref().x;
                                  other_line->transitive_line_start_ref().position_ref().y =
                                      e.transitive_location_ref().position_cref().y;
                                } else if (cascade_out.type() == cascade_type::end) {
                                  other_line->line_end_ref().position_ref().x = e.location_ref().position_cref().x;
                                  other_line->line_end_ref().position_ref().y = e.location_ref().position_cref().y;
                                  other_line->transitive_line_end_ref().position_ref().x =
                                      e.transitive_location_ref().position_cref().x;
                                  other_line->transitive_line_end_ref().position_ref().y =
                                      e.transitive_location_ref().position_cref().y;
                                }
                              }
                            }
                          },
                          [&](data_staging::line& l) {
                            handle_pass2(abstract_shape, l, l.meta_ref());
                          },
                          [&](data_staging::text& t) {
                            handle_pass2(abstract_shape, t, t.meta_ref());
                          },
                          [&](data_staging::script& s) {
                            handle_pass2(abstract_shape, s, s.meta_ref());
                          }},
               abstract_shape);
  }
}

bool interactor::destroy_if_duplicate(const std::string& unique_group, data_staging::shape_t& shape) {
  bool destroyed = false;
  // round x to the nearest 0.25 resolution x, y
  // auto& created_instance = instance;

  const auto destroy_shape = [&]() {
    gen_.destroy(shape);
    destroyed = true;
  };

  auto query_shape = [&](auto& shape) {
    if constexpr (std::is_same_v<std::decay_t<decltype(shape)>, data_staging::line>) {
      unique_groups[unique_group].query(destroy_shape,
                                        shape.transitive_line_start_ref().position_ref().x,
                                        shape.transitive_line_start_ref().position_ref().y,
                                        shape.transitive_line_end_ref().position_ref().x,
                                        shape.transitive_line_end_ref().position_ref().y);
    } else {
      unique_groups[unique_group].query(destroy_shape,
                                        shape.transitive_location_ref().position_ref().x,
                                        shape.transitive_location_ref().position_ref().y);
    }
  };

  meta_callback(shape, query_shape);
  return destroyed;
}

void interactor::update_object_toroidal(
    data_staging::toroidal& toroidal_data, double& x, double& y, double& diff_x, double& diff_y) {
  if (toroidal_data.group().empty()) return;

  auto the_width = toroidal_manager_.get(toroidal_data.group()).width;
  auto the_height = toroidal_manager_.get(toroidal_data.group()).height;
  auto x_offset = toroidal_manager_.get(toroidal_data.group()).x;
  auto y_offset = toroidal_manager_.get(toroidal_data.group()).y;

  diff_x = 0;
  diff_y = 0;

  auto box_top_left_x = x_offset - (the_width / 2);
  auto box_top_left_y = y_offset - (the_height / 2);
  auto box_bottom_right_x = x_offset + (the_width / 2);
  auto box_bottom_right_y = y_offset + (the_height / 2);

  // do the warp
  if (x < box_top_left_x) {
    x = box_bottom_right_x - (box_top_left_x - x);
    diff_x = the_width;
  }
  if (y < box_top_left_y) {
    y = box_bottom_right_y - (box_top_left_y - y);
    diff_y = the_height;
  }
  if (x > box_bottom_right_x) {
    x = box_top_left_x + (x - box_bottom_right_x);
    diff_x = -the_width;
  }
  if (y > box_bottom_right_y) {
    y = box_top_left_y + (y - box_bottom_right_y);
    diff_y = -the_height;
  }

  toroidal_data.set_warp_width(the_width);
  toroidal_data.set_warp_height(the_height);
  toroidal_data.set_warp_x(x_offset);
  toroidal_data.set_warp_y(y_offset);
}

void interactor::handle_collisions(data_staging::shape_t& shape) {
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
    auto x = c.transitive_location_ref().position_ref().x;
    auto y = c.transitive_location_ref().position_ref().y;
    auto unique_id = c.meta_cref().unique_id();

    if (c.meta_cref().id() == "balls") return;

    auto radius = c.radius();
    auto radiussize = c.radius_size();

    qts[collision_group].query(unique_id, circle(position(x, y), radius * 2.0, radiussize * 2.0), found);
    if (radiussize < 1000 /* todo create property of course */) {
      for (const auto& collide : found) {
        const auto unique_id2 = collide.userdata;
        auto& shape2 = gen_.object_lookup_.at(unique_id2);
        try {
          data_staging::circle& c2 = std::get<data_staging::circle>(shape2.get());
          if (c2.meta_cref().id() != "balls" && c.meta_cref().unique_id() != c2.meta_cref().unique_id()) {
            handle_collision(c, c2, shape, shape2.get());
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

void interactor::handle_collision(data_staging::circle& instance,
                                  data_staging::circle& instance2,
                                  data_staging::shape_t& shape,
                                  data_staging::shape_t& shape2) {
  auto& i = gen_.genctx->i();
  auto unique_id = instance.meta_cref().unique_id();
  auto unique_id2 = instance2.meta_cref().unique_id();
  auto last_collide = instance.behavior_ref().last_collide();

  auto x = instance.transitive_location_ref().position_cref().x;
  auto y = instance.transitive_location_ref().position_cref().y;

  auto x2 = instance2.transitive_location_ref().position_cref().x;
  auto y2 = instance2.transitive_location_ref().position_cref().y;

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
  // NOTE: the old code was doing a creation of a new instance, pass true as 2nd param to get() below to get back that
  // behavior
  auto find = definitions_.get(instance.meta_cref().id());
  if (find) {
    auto object_definition = *find;
    auto handle_collide_for_shape = [&](auto& c, auto& object_bridge, auto other_unique_id) {
      object_bridge->push_object(c);
      i.call_fun(object_definition, object_bridge->instance(), "collide", other_unique_id);
      object_bridge->pop_object();
    };
    auto callback_wrapper = [&]<typename T>(T& shape, int64_t unique_id) {
      if constexpr (std::is_same_v<T, data_staging::circle>) {
        return handle_collide_for_shape(shape, gen_.bridges_.circle(), unique_id);
      } else if constexpr (std::is_same_v<T, data_staging::ellipse>) {
        return handle_collide_for_shape(shape, gen_.bridges_.ellipse(), unique_id);
      } else if constexpr (std::is_same_v<T, data_staging::line>) {
        return handle_collide_for_shape(shape, gen_.bridges_.line(), unique_id);
      } else if constexpr (std::is_same_v<T, data_staging::text>) {
        return handle_collide_for_shape(shape, gen_.bridges_.text(), unique_id);
      } else if constexpr (std::is_same_v<T, data_staging::script>) {
        return handle_collide_for_shape(shape, gen_.bridges_.script(), unique_id);
      }
      throw abort_exception("unknown (undefined) object is ignored");
    };
    meta_callback(shape, [&]<typename T>(T& shape_concrete) {
      callback_wrapper(shape_concrete, unique_id2);
    });
    meta_callback(shape2, [&]<typename T>(T& shape_concrete) {
      callback_wrapper(shape_concrete, unique_id);
    });
  }
}

void interactor::handle_gravity(data_staging::shape_t& shape) {
  const auto handle = [&](auto& c) {
    std::vector<point_type> found;

    auto unique_id = c.meta_cref().unique_id();
    auto gravity_group = c.behavior_ref().gravity_group();
    if (gravity_group.empty()) {
      return;
    }

    if (c.movement_ref().velocity_speed() == 0) return;  // skip this one.

    auto x = c.transitive_location_ref().position_cref().x;
    auto y = c.transitive_location_ref().position_cref().y;

    auto radius = c.radius();
    auto radiussize = c.radius_size();

    auto& video = gen_.genctx->video_obj;
    auto& i = gen_.genctx->i();
    auto G = i.double_number(video, "gravity_G", 1);
    auto range = i.double_number(video, "gravity_range", 1000);
    const auto constrain_dist_min = i.double_number(video, "gravity_constrain_dist_min", 5.);
    const auto constrain_dist_max = i.double_number(video, "gravity_constrain_dist_max", 25.);

    qts_gravity[gravity_group].query(
        unique_id, circle(position(x, y), range + (radius * 2.0), range + (radiussize * 2.0)), found);

    vector2d acceleration(0, 0);
    const auto handle = [&](auto& c, auto& c2) {
      if (c.meta_cref().unique_id() != c2.meta_cref().unique_id()) {
        handle_gravity(c, c2, acceleration, G, range, constrain_dist_min, constrain_dist_max);
      }
    };
    for (const auto& in_range : found) {
      const auto unique_id2 = in_range.userdata;
      auto shape2 = gen_.object_lookup_.at(unique_id2);
      if (std::holds_alternative<data_staging::circle>(shape2.get())) {
        data_staging::circle& c2 = std::get<data_staging::circle>(shape2.get());
        handle(c, c2);
      }
      if (std::holds_alternative<data_staging::script>(shape2.get())) {
        data_staging::script& c2 = std::get<data_staging::script>(shape2.get());
        handle(c, c2);
      }
    }
    auto vel = add_vector(c.movement_ref().velocity(), acceleration);
    c.movement_ref().set_velocity(vel);
  };

  if (std::holds_alternative<data_staging::circle>(shape)) {
    data_staging::circle& c = std::get<data_staging::circle>(shape);
    handle(c);
  }
  if (std::holds_alternative<data_staging::script>(shape)) {
    data_staging::script& c = std::get<data_staging::script>(shape);
    handle(c);
  }
}

template <typename T, typename T2>
void interactor::handle_gravity(T& instance,
                                T2& instance2,
                                vector2d& acceleration,
                                double G,
                                double range,
                                double constrain_dist_min,
                                double constrain_dist_max) const {
  // auto unique_id = instance.meta_cref().unique_id();
  auto x = instance.transitive_location_ref().position_cref().x;
  auto y = instance.transitive_location_ref().position_cref().y;

  // auto unique_id2 = instance.meta_cref().unique_id();
  auto x2 = instance2.transitive_location_ref().position_cref().x;
  auto y2 = instance2.transitive_location_ref().position_cref().y;

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
  force = multiply_vector(force, strength / static_cast<double>(gen_.stepper.max_step));
  force = divide_vector(force, mass);

  acceleration.x += force.x;
  acceleration.y += force.y;
}
}  // namespace interpreter