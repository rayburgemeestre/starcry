/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmath>

#include "bitmap_wrapper.hpp"
#include "rendering_engine.h"

#include <fmt/core.h>

#ifndef EMSCRIPTEN
#include "util/image_utils.h"
#endif
#include "util/scope_exit.hpp"

using namespace std;

std::vector<util::rectangle<uint32_t>> create_rectangles(uint32_t width, uint32_t height, size_t scale_size);
void process_rectangles(image &target_bmp,
                        const std::vector<util::rectangle<uint32_t>> &rectangles,
                        const std::function<void(image &, uint32_t, uint32_t, uint32_t, uint32_t)> &exec_func);

#include "data/shape.hpp"
#include "draw_logic.hpp"
#include "image.hpp"
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
#include "starcry/metrics.h"
#endif
#endif

image rendering_engine::render(
    render_params &params, uint32_t offset_x, uint32_t offset_y, uint32_t width, uint32_t height) {
  image target_bmp;
  auto _exec =
      [&](image &target_bmp, uint32_t width, uint32_t height, uint32_t extra_offset_x, uint32_t extra_offset_y) {
        return _render(target_bmp, params, offset_x + extra_offset_x, offset_y + extra_offset_y, width, height);
      };
  // in case -c 0 is specified, we try to 'auto chunk' the image based on number of motion blur frames
  // the worst-case is having each pixel change, meaning memory usage multiplies the entire image size
  // times the number of motion blur frames.
  if (params.scales.size() <= 1 && params.num_chunks >= 1) {
    _exec(target_bmp, width, height, 0, 0);
    return target_bmp;
  }

  // auto chunking into rectangular chunks
  target_bmp.resize(static_cast<int>(width), static_cast<int>(height));
  auto rectangles = create_rectangles(width, height, params.scales.size());
  process_rectangles(target_bmp, rectangles, _exec);
  return target_bmp;
}

std::vector<util::rectangle<uint32_t>> create_rectangles(uint32_t width, uint32_t height, size_t scale_size) {
  util::ImageSplitter<uint32_t> imagesplitter{width, height};
  return imagesplitter.split(uint32_t(scale_size), util::ImageSplitter<uint32_t>::Mode::SplitHorizontal);
}

void process_rectangles(image &target_bmp,
                        const std::vector<util::rectangle<uint32_t>> &rectangles,
                        const std::function<void(image &, uint32_t, uint32_t, uint32_t, uint32_t)> &exec_func) {
  auto &bmp_pixels = target_bmp.pixels();
  size_t bmp_pixels_index = 0;
  for (const auto &rect : rectangles) {
    bitmap_wrapper bitmap;
    auto &tmp = bitmap.get(rect.width(), rect.height());
    exec_func(tmp, rect.width(), rect.height(), rect.x(), rect.y());
    auto &pixels = tmp.pixels();
    std::copy(pixels.begin(), pixels.end(), bmp_pixels.begin() + static_cast<long>(bmp_pixels_index));
    bmp_pixels_index += pixels.size();
  }
}

double to_abs_x(double x, double center_x, double scale, double offset_x, double canvas_w) {
  return ((x - center_x) * scale) - offset_x + canvas_w / 2;
}

double to_abs_y(double y, double center_y, double scale, double offset_y, double canvas_h) {
  return ((y - center_y) * scale) - offset_y + canvas_h / 2;
}

void rendering_engine::_render(
    image &target_bmp, render_params &params, uint32_t offset_x, uint32_t offset_y, uint32_t width, uint32_t height) {
  if (!draw_logic_) {
    draw_logic_ = std::make_shared<draw_logic::draw_logic>();
  }
  target_bmp.resize(width, height);

  target_bmp.clear_to_color(params.bg_color);

  double scale_ratio = params.canvas_h / 1080.;  //(top_scale / 1080.) * canvas_h;  // std::min(canvas_w, canvas_h);
  if (!params.settings.scale_ratio) {
    scale_ratio = 1;
  }

  draw_logic_->canvas_width(params.canvas_w);
  draw_logic_->canvas_height(params.canvas_h);
  draw_logic_->height(height);
  draw_logic_->width(width);
  draw_logic_->height(height);
  draw_logic_->center(params.view_x, params.view_y);
  draw_logic_->offset(offset_x, offset_y);

  if (params.shapes.empty()) return;

#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
  params.metrics->resize_job_objects(
      params.thread_num, params.job_num, params.chunk_num, params.shapes[params.shapes.size() - 1].size());
#endif
#endif

  if (params.debug) {
    double r = double(offset_y) / double(params.canvas_h), g = 0, b = 0, a = 1;
    for (auto i = uint32_t(0); i < width; i++) {
      target_bmp.set(i, 0, r, g, b, a);
    }
    for (auto i = uint32_t(0); i < height; i++) {
      target_bmp.set(width / 2, i, r, g, b, a);
    }
    scope_exit se([&]() {
      const auto ct = [&](double offset_y, const std::string &text) {
        data::shape shape;  // = shape;
        shape.x = 0;
        shape.y = offset_y;
        shape.r = 1;
        shape.g = 1;
        shape.b = 1;
        shape.opacity = 1;
        shape.type = data::shape_type::text;
        shape.text = text;
        shape.text_fixed = true;
        shape.text_size = 15;
        shape.align = "left";
        shape.gradient_id_str = "";
        shape.gradients_.emplace_back(1.0, data::gradient{});
        shape.gradients_[0].second.colors.emplace_back(std::make_tuple(0.0, data::color{1.0, 1, 1, 1}));
        shape.gradients_[0].second.colors.emplace_back(std::make_tuple(1.0, data::color{1.0, 1, 1, 1}));
        shape.texture_id_str = "";
        shape.blending_ = data::blending_type::normal;
        return shape;
      };

      draw_logic_->render_text(target_bmp,
                               ct(0, fmt::format("canvas: ({}, {})", params.canvas_w, params.canvas_h)),
                               1.,
                               params.settings,
                               true);
      draw_logic_->render_text(
          target_bmp, ct(20, fmt::format("size: ({}, {})", width, height)), 1., params.settings, true);
      draw_logic_->render_text(
          target_bmp, ct(60, fmt::format("offset: ({}, {})", offset_x, offset_y)), 1., params.settings, true);
      draw_logic_->render_text(
          target_bmp, ct(80, fmt::format("view: ({}, {})", params.view_x, params.view_y)), 1., params.settings, true);
      draw_logic_->render_text(
          target_bmp, ct(100, fmt::format("scale: {}", params.top_scale)), 1., params.settings, true);
      draw_logic_->render_text(
          target_bmp, ct(120, fmt::format("scale ratio: {}", scale_ratio)), 1., params.settings, true);
    });
  }

  if (!params.shapes.empty()) {
    for (const auto &shape : params.shapes[params.shapes.size() - 1]) {
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
      double index = 0;
      params.metrics->set_render_job_object_state(
          params.thread_num, params.job_num, params.chunk_num, index, metrics::job_state::rendering);
#endif
#endif
      if (!params.selected_ids.empty()) {
        // TODO: selected_ids should be a set, although for now perhaps this is fine, since we're only dealing with
        // around 3 or 4 items in selected_ids most likely.
        // EDIT: this is no longer the case, we build up a transitive selection
        if (std::find(params.selected_ids.begin(), params.selected_ids.end(), shape.unique_id) ==
            params.selected_ids.end()) {
          continue;
        }
      }

      if (!params.scales.empty()) {
        draw_logic_->capture_pixels(true);

        double opacity = 1.0;
        const auto scale = params.scales[params.scales.size() - 1] /* * scale_ratio */;
        draw_logic_->scale(scale * scale_ratio);
        // invoke first
        auto box =
            invoke_render(target_bmp, params, shape, opacity, params.settings, scale, scale_ratio, offset_x, offset_y);

        // invoke rest
        for (const auto &index_data : shape.indexes) {
          const auto &step = index_data.first;
          const auto &index = index_data.second;
          const auto &shape = params.shapes[step][index];
          const auto scale = params.scales[step] /* * scale_ratio */;
          draw_logic_->scale(scale * scale_ratio);
          box.update(invoke_render(
              target_bmp, params, shape, opacity, params.settings, scale, scale_ratio, offset_x, offset_y));
        }
        box.normalize(width, height);
        if (params.debug) draw_logic_->render_bounding_box(target_bmp, box);

        draw_logic_->capture_pixels(false);

        draw_captured_pixels(target_bmp, width, height, shape);
        // if (debug) draw_logic_->render_bounding_box(target_bmp, box);
      }
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
      params.metrics->set_render_job_object_state(
          params.thread_num, params.job_num, params.chunk_num, index, metrics::job_state::rendered);
      index++;
#endif
#endif
    }
    return;
  }
}
bounding_box rendering_engine::invoke_render(image &target_bmp,
                                             render_params &params,
                                             const auto &shape,
                                             double opacity,
                                             const data::settings &settings,
                                             double scale,
                                             double scale_ratio,
                                             double offset_x,
                                             double offset_y) {
  bounding_box box;

  const auto invoke = [&]() {
    switch (shape.type) {
      case data::shape_type::circle:
        box = draw_logic_->render_circle(target_bmp, shape, opacity, settings);
        break;
      case data::shape_type::ellipse:
        box = draw_logic_->render_ellipse(target_bmp, shape, opacity, settings);
        break;
      case data::shape_type::line:
        // no box yet
        draw_logic_->render_line(target_bmp, shape, opacity, settings);
        break;
      case data::shape_type::text:
        box = draw_logic_->render_text(target_bmp, shape, opacity, settings);
        break;
      case data::shape_type::script:
      case data::shape_type::none:
        break;
    }
  };
  invoke();

  if (shape.warp_width || shape.warp_height) {
    bounding_box toroidal_box;
    toroidal_box.update_x(to_abs_x(
        shape.warp_x - (shape.warp_width / 2.), params.view_x, scale * scale_ratio, offset_x, params.canvas_w));
    toroidal_box.update_x(to_abs_x(
        shape.warp_x + (shape.warp_width / 2.), params.view_x, scale * scale_ratio, offset_x, params.canvas_w));
    toroidal_box.update_y(to_abs_y(
        shape.warp_y - (shape.warp_height / 2.), params.view_y, scale * scale_ratio, offset_y, params.canvas_h));
    toroidal_box.update_y(to_abs_y(
        shape.warp_y + (shape.warp_height / 2.), params.view_y, scale * scale_ratio, offset_y, params.canvas_h));

    auto [left, right, top, bottom] = toroidal_box.collides(box);
    if (left) {
      draw_logic_->center(params.view_x - shape.warp_width, params.view_y);
      invoke();
      draw_logic_->center(params.view_x, params.view_y);
    }
    if (right) {
      draw_logic_->center(params.view_x + shape.warp_width, params.view_y);
      invoke();
      draw_logic_->center(params.view_x, params.view_y);
    }
    if (top) {
      draw_logic_->center(params.view_x, params.view_y - shape.warp_height);
      invoke();
      draw_logic_->center(params.view_x, params.view_y);
    }
    if (bottom) {
      draw_logic_->center(params.view_x, params.view_y + shape.warp_height);
      invoke();
      draw_logic_->center(params.view_x, params.view_y);
    }
  }
  return box;
}

void rendering_engine::draw_captured_pixels(image &target_bmp,
                                            uint32_t width,
                                            uint32_t height,
                                            const data::shape &shape) {
  auto &ref = draw_logic_->motionblur_buf();
  ref.set_layers(shape.indexes.size());
  for (const auto &p : ref.buffer()) {
    // These clamps should be avoided, and in draw_logic we should make sure we don't draw outside bounds!
    const auto &y = math::clamp(p.first, 0, (int)height);  // TODO: comment
    // const auto &y = p.first;
    for (const auto &q : p.second) {
      const auto &x = math::clamp(q.first, 0, (int)width);  // TODO: comment
      // const auto &x = q.first;
      const auto &color_dat = q.second;
      const auto col = ref.get_color(color_dat);
      // TODO: design is suffering a bit, draw_logic_ needs a refactoring
      draw_logic_->blend_the_pixel(target_bmp, shape.blending_.type(), x, y, col.a, col);
    }
  }
}

void rendering_engine::write_image(image &bmp, int width, int height, std::string filename) {
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
  png::image<png::rgba_pixel> image(width, height);
  copy_to_png(rand_, bmp.pixels(), width, height, image, false);
  image.write(filename);
#endif
#endif
}
