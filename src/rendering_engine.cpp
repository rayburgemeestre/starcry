/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmath>

#include "bitmap_wrapper.hpp"
#include "rendering_engine.h"
#include "rendering_engine/debug.h"
#include "util/motionblur_buffer_flat_rev.hpp"

#include <fmt/core.h>

#ifndef EMSCRIPTEN
#include "util/image_utils.h"
#endif

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

  // Force no auto-chunking
  // { _exec(target_bmp, width, height, 0, 0); return target_bmp; }

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
  params.metrics_->resize_job_objects(
      params.thread_num, params.job_num, params.chunk_num, params.shapes[params.shapes.size() - 1].size());
#endif
#endif

  if (params.debug) {
    draw_debug(target_bmp, draw_logic_, params, offset_x, offset_y, width, height, scale_ratio);
  }

  if (!params.shapes.empty()) {
    for (const auto &shape : params.shapes[params.shapes.size() - 1]) {
#ifndef EMSCRIPTEN
#ifndef SC_CLIENT
      double index = 0;
      params.metrics_->set_render_job_object_state(
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
      params.metrics_->set_render_job_object_state(
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

  // "motionblur_buffer" (lowest memory)
  // ref.draw_callback([this, &target_bmp, &shape](int x, int y, const data::color& col) -> void {
  //     draw_logic_->blend_the_pixel(target_bmp, shape.blending_.type(), x, y, col.a, col);
  // });

  // "motionblur_buffer_flat" (implementation uses too much memory)
  // ref.draw_callback([this, &target_bmp, &shape](int x, int y, const data::color& col) -> void {
  //  draw_logic_->blend_the_pixel(target_bmp, shape.blending_.type(), x, y, col.a, col);
  // });

  // "motionblur_buffer_flat_rev" (slightly more memory for faster memory access)
  ref.draw_callback(
      [this, &target_bmp, &shape](const motionblur_buffer_flat_rev::pixel_data &data, const data::color &col) -> void {
        draw_logic_->blend_the_pixel(target_bmp, shape.blending_.type(), data.x, data.y, col.a, col);
      });
}