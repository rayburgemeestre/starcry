#include <SDL2/SDL.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <cstdlib>

#include "data/job.hpp"
#include "data/shape.hpp"

float lerp(float a, float b, float t) {
  return (1 - t) * a + t * b;
}

struct context {
  SDL_Renderer *renderer;
  int iteration;
};

void mainloop(void *arg) {
  data::job job;
  memset(&job, 0x00, sizeof(job));
  job.width = 1920;
  job.height = 1080;
  job.canvas_w = 1920;
  job.canvas_h = 1080;
  job.scale = 1;

  data::shape circle;
  memset(&circle, 0x00, sizeof(circle));
  circle.radius = 50;
  circle.radius_size = 5;
  data::gradient gradient;
  data::color color1;
  color1.r = 1;
  color1.g = 0;
  color1.b = 0;
  color1.a = 1;
  data::color color2;
  color2.r = 0;
  color2.g = 0;
  color2.b = 1;
  color2.a = 0;
  gradient.colors.emplace_back(std::make_tuple(0.0, color1));
  gradient.colors.emplace_back(std::make_tuple(1.0, color2));
  circle.gradient_ = gradient;
  job.shapes.push_back(circle);


  context *ctx = static_cast<context *>(arg);
  SDL_Renderer *renderer = ctx->renderer;

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_Rect r;
  r.x = ctx->iteration % 1920;
  r.y = 50;
  r.w = 50;
  r.h = 50;
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  SDL_RenderFillRect(renderer, &r);

  SDL_RenderPresent(renderer);

  ctx->iteration++;
  emscripten_cancel_main_loop();
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
  emscripten_set_main_loop_arg(mainloop, &ctx, fps, simulate_infinite_loop);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("lerp", &lerp);
  emscripten::function("start", &start);
}

int main() {
  return EXIT_SUCCESS;
}
