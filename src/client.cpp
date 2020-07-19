#include <SDL2/SDL.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "data/job.hpp"
#include "data/shape.hpp"

float lerp(float a, float b, float t) {
  return (1 - t) * a + t * b;
}

struct context {
  SDL_Renderer *renderer;
  int iteration;
};

#include "bitmap_wrapper.hpp"
#include "data/pixels.hpp"
#include "primitives/color.cpp"
#include "rendering_engine_wrapper.cpp"
#include "rendering_engine_wrapper.h"

std::chrono::high_resolution_clock::time_point begin_time, end_time;
rendering_engine_wrapper engine;
bitmap_wrapper bitmap;
data::job job;
std::vector<uint32_t> transfer_pixels;

int main() {
  engine.initialize();

  memset(&job, 0x00, sizeof(job));
  job.width = 1920;
  job.height = 1080;
  job.canvas_w = 1920;
  job.canvas_h = 1080;
  job.scale = 1;

  data::shape circle;
  memset(&circle, 0x00, sizeof(circle));
  circle.radius = 500;
  circle.radius_size = 500;
  data::gradient gradient;
  data::color color1;
  color1.r = 1;
  color1.g = 0;
  color1.b = 0;
  color1.a = 0.5;
  data::color color2;
  color2.r = 0;
  color2.g = 0;
  color2.b = 1;
  color2.a = 1.0;
  gradient.colors.emplace_back(std::make_tuple(0.0, color1));
  gradient.colors.emplace_back(std::make_tuple(1.0, color2));
  circle.gradient_ = gradient;
  circle.x = 0;
  circle.y = 0;
  circle.z = 0;
  circle.type = data::shape_type::circle;
  job.shapes.push_back(circle);

  return EXIT_SUCCESS;
}

void mainloop(void *arg) {
  auto &bmp = bitmap.get(job.width, job.height);
  engine.render(bmp,
                job.background_color,
                job.shapes,
                job.offset_x,
                job.offset_y,
                job.canvas_w,
                job.canvas_h,
                job.width,
                job.height,
                job.scale);

  context *ctx = static_cast<context *>(arg);
  SDL_Renderer *renderer = ctx->renderer;

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, job.width, job.height);

  auto &pixels = bmp.pixels();
  transfer_pixels.reserve(pixels.size());
  for (const auto &pix : pixels) {
    uint32_t color;
    char *cptr = (char *)&color;
    *cptr = (char)(pix.r * 255);
    cptr++;
    *cptr = (char)(pix.g * 255);
    cptr++;
    *cptr = (char)(pix.b * 255);
    cptr++;
    *cptr = (char)(pix.a * 255);
    transfer_pixels.push_back(color);
  }

  SDL_UpdateTexture(texture, NULL, (void *)&(transfer_pixels[0]), 1920 * sizeof(Uint32));

  //  SDL_Rect r;
  //  r.x = ctx->iteration % 1920;
  //  r.y = 50;
  //  r.w = 50;
  //  r.h = 50;
  //  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  //  SDL_RenderFillRect(renderer, &r);
  SDL_RenderCopy(renderer, texture, NULL, NULL);

  SDL_RenderPresent(renderer);

  end_time = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> idle = end_time - begin_time;

  std::cout << "Frame: " << ctx->iteration << " " << ((double)ctx->iteration / (idle.count() / 1000)) << std::endl;
  ctx->iteration++;
  // emscripten_cancel_main_loop();
}

void start() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer(1920, 1080, 0, &window, &renderer);

  context ctx;
  ctx.renderer = renderer;
  ctx.iteration = 0;

  const int simulate_infinite_loop = 1;  // call the function repeatedly
  const int fps = -1;                    // call the function as fast as the browser wants to render (typically 60fps)

  begin_time = std::chrono::high_resolution_clock::now();

  emscripten_set_main_loop_arg(mainloop, &ctx, fps, simulate_infinite_loop);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("lerp", &lerp);
  emscripten::function("start", &start);
}
