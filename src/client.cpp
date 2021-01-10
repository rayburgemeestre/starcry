#include <SDL2/SDL.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/bind.h>
#endif
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "cereal/archives/json.hpp"

#include "data/job.hpp"
#include "data/shape.hpp"

#include "bitmap_wrapper.hpp"
// #include "primitives/color.cpp"
#include "rendering_engine_wrapper.cpp"
#include "rendering_engine_wrapper.h"

bool initialized = false;
std::chrono::high_resolution_clock::time_point begin_time, end_time;
rendering_engine_wrapper engine;
bitmap_wrapper bitmap;
data::job job;
std::vector<uint32_t> transfer_pixels;
SDL_Texture *texture = nullptr;
SDL_Renderer *renderer = nullptr;
int x = 0, y = 0;

struct context {
  SDL_Renderer *renderer;
  int iteration;
};

void mainloop(void *arg) {
#ifdef EMSCRIPTEN
  context *ctx = static_cast<context *>(arg);
  renderer = ctx->renderer;
#else
  renderer = static_cast<SDL_Renderer *>(arg);
#endif

  SDL_SetRenderDrawColor(renderer, 0, 0, 67, 255);
  SDL_RenderClear(renderer);

  SDL_Rect r, r2;
  SDL_PumpEvents();
  SDL_GetMouseState(&x, &y);
  r.x = 0;
  r.y = y;
  r2.x = x;
  r2.y = 0;
  r.w = job.width;
  r.h = 1;
  r2.w = 1;
  r2.h = job.height;

  SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderFillRect(renderer, &r);
  SDL_RenderFillRect(renderer, &r2);

  SDL_RenderPresent(renderer);

  end_time = std::chrono::high_resolution_clock::now();

  // std::chrono::duration<double, std::milli> idle = end_time - begin_time;

#ifdef EMSCRIPTEN
  // std::cout << "Frame: " << ctx->iteration << " " << ((double)ctx->iteration / (idle.count() / 1000)) << std::endl;
  ctx->iteration++;
  // emscripten_cancel_main_loop();
#else
  // std::cout << "Frame: " << i << " " << ((double)i / (idle.count() / 1000)) << std::endl;
#endif
}

void render_shapes_to_texture() {
  auto &bmp = bitmap.get(job.width, job.height);
  data::settings settings_;
  std::shared_ptr<metrics> tmp = nullptr;
  engine.render(0,
                job.job_number == std::numeric_limits<uint32_t>::max() ? job.frame_number : job.job_number,
                job.chunk,
                tmp,
                bmp,
                job.background_color,
                job.shapes,
                job.offset_x,
                job.offset_y,
                job.canvas_w,
                job.canvas_h,
                job.width,
                job.height,
                job.scale,
                job.scales,
                false,
                settings_);
  if (texture == nullptr && renderer == nullptr) {
    return;
  }
  if (texture != nullptr) {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, job.width, job.height);

  auto &pixels = bmp.pixels();
  transfer_pixels.clear();
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
  SDL_UpdateTexture(texture, NULL, (void *)&(transfer_pixels[0]), job.width * sizeof(Uint32));
}

void initialize(uint32_t width, uint32_t height) {
  engine.initialize();

  memset(&job, 0x00, sizeof(job));
  job.width = width;
  job.height = height;
  job.canvas_w = width;
  job.canvas_h = height;
  job.scale = 1;

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
}

void start(uint32_t width, uint32_t height) {
  if (!initialized) {
    initialize(width, height);
  }

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
#ifdef EMSCRIPTEN
  SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
  SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
  SDL_EventState(SDL_KEYUP, SDL_DISABLE);

  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
  context ctx;
  ctx.renderer = renderer;
  ctx.iteration = 0;
#else
  window = SDL_CreateWindow(
      "SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, job.width, job.height, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "could not create window: %s\n", SDL_GetError());
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

#endif

  begin_time = std::chrono::high_resolution_clock::now();

#ifdef EMSCRIPTEN
  const int simulate_infinite_loop = 1;  // call the function repeatedly
  const int fps = -1;                    // call the function as fast as the browser wants to render (typically 60fps)
  emscripten_set_main_loop_arg(mainloop, &ctx, fps, simulate_infinite_loop);
  SDL_DestroyRenderer(renderer);
#else
  render_shapes_to_texture();
  while (true) {
    /*
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        break;
      }
    }
    */
    mainloop((void *)renderer);
  }
#endif

  SDL_DestroyWindow(window);
  SDL_Quit();
}

void set_shapes(std::string data) {
  std::istringstream is(data);
  cereal::JSONInputArchive archive(is);
  // cereal::BinaryInputArchive archive(is);
  data::job tmp;
  archive(tmp);
  std::swap(job, tmp);
  is.str("");
  is.clear();

  render_shapes_to_texture();
}

void set_texture(std::string data) {
  if (texture == nullptr && renderer == nullptr) {
    return;
  }
  if (texture != nullptr) {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, job.width, job.height);
  // todo, hardcoded
  SDL_UpdateTexture(texture, NULL, (void *)&(data[0]), job.width * sizeof(Uint32));
}

int get_mouse_x() {
  return x;
}
int get_mouse_y() {
  return x;
}
double get_scale() {
  return job.scale;
}

int main() {
#ifndef EMSCRIPTEN
  start(1920, 1080);
#endif
  return EXIT_SUCCESS;
}

#ifdef EMSCRIPTEN
EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("start", &start);
  emscripten::function("set_shapes", &set_shapes);
  emscripten::function("set_texture", &set_texture);
  emscripten::function("get_mouse_x", &get_mouse_x);
  emscripten::function("get_mouse_y", &get_mouse_y);
  emscripten::function("get_scale", &get_scale);
}
#endif
