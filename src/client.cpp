#include <SDL2/SDL.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/bind.h>
#endif
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <vector>

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include "nlohmann/json.hpp"

#include "data/job.hpp"
#include "data/shape.hpp"

#include "rendering_engine.cpp"
#include "rendering_engine.h"

bool initialized = false;
std::chrono::high_resolution_clock::time_point begin_time, end_time;
rendering_engine engine;
data::job job;
std::vector<uint32_t> transfer_pixels;
std::map<int, std::tuple<SDL_Rect, SDL_Rect, SDL_Texture *>> textures;  // by chunk
SDL_Texture *texture = nullptr;
int texture_w = 0;
int texture_h = 0;
int chunk = 1;
int num_chunks = 1;
int last_num_chunks = 1;
SDL_Renderer *renderer = nullptr;
int x = 0, y = 0;
bool pointer_state = true;
auto last_received = std::chrono::high_resolution_clock::now();
auto wake_for = 1000;  // milliseconds

void wake_main_loop() {
#ifdef EMSCRIPTEN
  // wake up main loop
  emscripten_resume_main_loop();
  last_received = std::chrono::high_resolution_clock::now();
#endif
}

struct client_context {
  SDL_Renderer *renderer;
  int iteration;
};

void render_shapes_to_texture();

void mainloop(void *arg) {
#ifdef EMSCRIPTEN
  client_context *ctx = static_cast<client_context *>(arg);
  renderer = ctx->renderer;
#else
  renderer = static_cast<SDL_Renderer *>(arg);
#endif

  SDL_SetRenderDrawColor(renderer, 0, 0, 67, 255);
  SDL_RenderClear(renderer);

  SDL_Rect r, r2;
  // turns out there is an issue in emscripten newer versions
  // that screws up onkeydown events etc., this ifndef didn't help
#ifndef EMSCRIPTEN
  SDL_PumpEvents();
#endif
  SDL_GetMouseState(&x, &y);
  r.x = 0;
  r.y = y;
  r2.x = x;
  r2.y = 0;
  r.w = job.canvas_w;
  r.h = 1;
  r2.w = 1;
  r2.h = job.canvas_h;

  SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);

#ifndef EMSCRIPTEN
  // TODO: this function, is not playing along very well with newer code!!
  render_shapes_to_texture();
#endif

  double ratio_w = double(job.canvas_w) / double(texture_w);
  // double ratio_h = double(job.canvas_h) / double(texture_h);
  double ratio = ratio_w;  // std::min(ratio_w, ratio_h);

  // SDL_RenderCopy(renderer, texture, &texture_source_rect, &texture_rect);
  // This will not resize, so if it's too big or small, just put it at 0,0
  // SDL_RenderCopy(renderer, texture, &texture_source_rect, &texture_rect);

  for (const auto &tex : textures) {
    SDL_RenderCopy(renderer, std::get<2>(tex.second), &std::get<0>(tex.second), &std::get<1>(tex.second));
    // Draw box around the texture rectangles
    // SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    // SDL_RenderDrawRect(renderer, &std::get<1>(tex.second));
    // SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    // SDL_RenderDrawRect(renderer, &std::get<1>(tex.second));
  }

  if (pointer_state) {
    SDL_RenderFillRect(renderer, &r);
    SDL_RenderFillRect(renderer, &r2);
  }

  SDL_RenderPresent(renderer);

  end_time = std::chrono::high_resolution_clock::now();

  // std::chrono::duration<double, std::milli> idle = end_time - begin_time;

#ifdef EMSCRIPTEN
  // std::cout << "Frame: " << ctx->iteration << " " << ((double)ctx->iteration / (idle.count() / 1000)) << std::endl;
  ctx->iteration++;

  // pause mainloop after 100 ms have passed
  const auto current_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> idle = current_time - last_received;
  if (idle.count() > wake_for) {
    emscripten_pause_main_loop();
  }

#else
  // std::cout << "Frame: " << i << " " << ((double)i / (idle.count() / 1000)) << std::endl;
#endif
}

void render_shapes_to_texture() {
  data::settings settings_;
  std::shared_ptr<metrics> tmp = nullptr;
  image bmp;
  render_params params{
      0,
      job.job_number == std::numeric_limits<uint32_t>::max() ? job.frame_number : job.job_number,
      job.chunk,
      job.num_chunks,
      tmp,
      job.background_color,
      job.shapes,
      0,
      0,
      job.canvas_w,
      job.canvas_h,
      job.scale,
      job.scales,
      false,
      settings_,
      false /* debug */,
      {},
  };
  bmp = engine.render(params, job.offset_x, job.offset_y, job.width, job.height);
  if (texture == nullptr && renderer == nullptr) {
    return;
  }
  if (texture != nullptr) {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, job.width, job.height);
  texture_w = job.width;
  texture_h = job.height;

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

void initialize(uint32_t width, uint32_t height, uint32_t canvas_w, uint32_t canvas_h) {
  memset(&job, 0x00, sizeof(job));
  job.width = width;
  job.height = height;
  job.width = canvas_w;
  job.height = canvas_h;
  job.canvas_w = canvas_w;
  job.canvas_h = canvas_h;
  job.scale = 1;
  job.scales = {1.};

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
  gradient.colors.emplace_back(0.0, color1);
  gradient.colors.emplace_back(1.0, color2);

#ifndef EMSCRIPTEN
  data::shape circle;
  circle.gradients_.emplace_back(1.0, gradient);
  circle.x = 0;
  circle.y = 0;
  circle.z = 0;
  circle.radius = 600;
  circle.radius_size = 600;
  circle.blending_ = data::blending_type::normal;
  circle.opacity = 1.;
  circle.scale = 1.;
  circle.type = data::shape_type::circle;
  if (job.shapes.empty()) {
    job.shapes.emplace_back();
  }
  job.shapes[0].push_back(circle);
#endif
}

client_context ctx;

void start(uint32_t width, uint32_t height, uint32_t canvas_w, uint32_t canvas_h) {
  printf("initializing with: %dx%d, %dx%d\n", (int)width, (int)height, (int)canvas_w, (int)canvas_h);
  initialize(width, height, canvas_w, canvas_h);

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
#ifdef EMSCRIPTEN
  SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
  SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
  SDL_EventState(SDL_KEYUP, SDL_DISABLE);

  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer(canvas_w, canvas_h, 0, &window, &renderer);
  ctx.renderer = renderer;
  ctx.iteration = 0;
#else
  window = SDL_CreateWindow(
      "SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, job.width, job.height, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "could not create window: %s\n", SDL_GetError());
    return;
  }

  // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

#endif

  begin_time = std::chrono::high_resolution_clock::now();

#ifdef EMSCRIPTEN
  const int simulate_infinite_loop = 1;  // call the function repeatedly
  const int fps = -1;                    // call the function as fast as the browser wants to render (typically 60fps)
  emscripten_set_main_loop_arg(mainloop, &ctx, fps, simulate_infinite_loop);
  SDL_DestroyRenderer(renderer);
#else
  // TODO: why is this function here?
  render_shapes_to_texture();
  while (true) {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYUP) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          std::exit(0);
        }
      }
      if (event.type == SDL_QUIT) {
        break;
      }
    }
    mainloop((void *)renderer);
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);
    // this is a test
    job.width = width;
    job.height = height;
    job.canvas_w = width;
    job.canvas_h = height;
  }
#endif

  SDL_DestroyWindow(window);
  SDL_Quit();
}

void stop() {
#ifdef EMSCRIPTEN
  emscripten_cancel_main_loop();
#endif
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

  wake_main_loop();
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
  wake_main_loop();
  size_t n = data.size() - 16;
  char *ptr = &(data[0]);
  memcpy(&texture_w, ptr + n, sizeof(uint32_t));
  memcpy(&texture_h, ptr + n + sizeof(uint32_t), sizeof(uint32_t));
  memcpy(&chunk, ptr + n + sizeof(uint32_t) * 2, sizeof(uint32_t));
  memcpy(&num_chunks, ptr + n + sizeof(uint32_t) * 3, sizeof(uint32_t));

  if (num_chunks != last_num_chunks) {
    // clear all textures first
    for (auto &tex : textures) {
      SDL_DestroyTexture(std::get<2>(tex.second));
    }
    textures.clear();
  }
  last_num_chunks = num_chunks;

  printf("texture_w: %d, texture_h: %d, chunk: %d, num_chunks: %d\n", texture_w, texture_h, chunk, num_chunks);
  printf("data size: %d\n", (int)data.size());
  printf("debug pixel: %d\n", (int)data[0]);

  // chunk texture
  if (textures.find(chunk) != textures.end()) {
    SDL_DestroyTexture(std::get<2>(textures[chunk]));
    textures.erase(chunk);
  }

  std::get<0>(textures[chunk]) = {0, 0, texture_w, texture_h};
  std::get<1>(textures[chunk]) = {0, (chunk - 1) * texture_h, texture_w, texture_h};
  std::get<2>(textures[chunk]) =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, texture_w, texture_h);
  SDL_UpdateTexture(std::get<2>(textures[chunk]), NULL, (void *)(ptr + 16), texture_w * sizeof(Uint32));

  // we have the complicated issue that the last chunk can be more than the others in terms of height,
  // e.g., with chunks 1, 2, 3, 4 (num_chunks 4), 10, 10, 10, 15 (if there are 5 remaining pixels)
  // then the last texture will be positioned at (3 * 15 =) 45, which is 5 pixels too far (should be 3 x 10)

  // we will now update the last chunk with any of the other chunks (if there are any)
  auto last_chunk = textures.find(num_chunks);
  if (last_chunk != textures.end()) {
    // first find any other chunk, that is not the last one
    for (auto &tex : textures) {
      if (tex.first == num_chunks) {
        continue;
      }
      std::get<1>(last_chunk->second).y = (num_chunks - 1) * std::get<0>(tex.second).h;
      break;  // success
    }
  }

  // This intermediate texture is no longer used, and we cannot do resizes anymore
  // might delete this logic completely at some point.
  texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, texture_w, texture_h * num_chunks);
}

int get_mouse_x() {
  wake_main_loop();
  return x;
}
int get_mouse_y() {
  wake_main_loop();
  return y;
}

int get_canvas_w() {
  return job.canvas_w;
}
int get_canvas_h() {
  return job.canvas_h;
}

int get_texture_w() {
  return texture_w;
}
int get_texture_h() {
  return texture_h;
}

double get_scale() {
  return job.scale;
}
void toggle_pointer() {
  wake_main_loop();
  pointer_state = !pointer_state;
}

std::vector<int> get_gradient_colors(std::string gradient_definition, int width) {
  const auto json_obj = nlohmann::json::parse(gradient_definition);
  data::gradient converter;
  if (json_obj.is_string()) {
    const auto gradient_short_hand = json_obj.get<std::string>();
    auto color_string = gradient_short_hand;
    auto r = std::stoi(color_string.substr(1, 2), nullptr, 16) / 255.;
    auto g = std::stoi(color_string.substr(3, 2), nullptr, 16) / 255.;
    auto b = std::stoi(color_string.substr(5, 2), nullptr, 16) / 255.;
    double index = 0.9;
    if (color_string.length() >= 8 && color_string[7] == '@') {
      // String is for example '#ffffff@0.9', keep only the 0.9 part
      try {
        auto remainder = std::stod(color_string.substr(8));
        index = std::clamp(remainder, 0.0, 1.0);
      } catch (const std::invalid_argument &e) {
        logger(DEBUG) << "Invalid argument: " << e.what() << std::endl;
      } catch (const std::out_of_range &e) {
        logger(DEBUG) << "Out of range: " << e.what() << std::endl;
      }
    }
    converter.colors.emplace_back(0.0, data::color{r, g, b, 1.});
    converter.colors.emplace_back(index, data::color{r, g, b, 1.});
    converter.colors.emplace_back(1.0, data::color{r, g, b, 0.});
  } else {
    for (const auto &color_line : json_obj) {
      const auto pos = color_line["position"];
      const auto r = color_line["r"];
      const auto g = color_line["g"];
      const auto b = color_line["b"];
      const auto a = color_line["a"];
      converter.colors.emplace_back(pos, data::color{r, g, b, a});
    }
  }
  std::vector<int> colors;
  for (int i = 0; i < width; ++i) {
    const auto clr = converter.get(i / (double)width);
    colors.emplace_back(clr.r * 255);
    colors.emplace_back(clr.g * 255);
    colors.emplace_back(clr.b * 255);
    colors.emplace_back(clr.a * 255);
  }
  return colors;
}

// NOTE: currently dead code.
std::string shapes_binary_to_json(const std::string &binary) {
  std::istringstream is(binary);
  // tried binary first
  cereal::JSONInputArchive archive(is);
  std::vector<data::shape> shapes;
  try {
    archive(shapes);  // crashes in wasm
  } catch (std::exception &e) {
    std::cout << "exception during archive: " << e.what() << std::endl;
    throw e;
  } catch (...) {
    std::cout << "unexpected exception during archive" << std::endl;
    throw;
  }
  std::string json_output;
  if (!data::shapes_to_json(shapes, json_output)) {
    std::cout << "failed to convert" << std::endl;
  }
  return json_output;
}

int main() {
#ifndef EMSCRIPTEN
  start(1080, 1080, 1080, 1080);
#endif
  return EXIT_SUCCESS;
}

#ifdef EMSCRIPTEN
EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::register_vector<int>("VectorInt");
  emscripten::function("start", &start);
  emscripten::function("stop", &stop);
  emscripten::function("set_shapes", &set_shapes);
  emscripten::function("set_texture", &set_texture);
  emscripten::function("get_mouse_x", &get_mouse_x);
  emscripten::function("get_mouse_y", &get_mouse_y);
  emscripten::function("get_canvas_w", &get_canvas_w);
  emscripten::function("get_canvas_h", &get_canvas_h);
  emscripten::function("get_texture_w", &get_texture_w);
  emscripten::function("get_texture_h", &get_texture_h);
  emscripten::function("get_gradient_colors", &get_gradient_colors);
  emscripten::function("get_scale", &get_scale);
  emscripten::function("toggle_pointer", &toggle_pointer);
  emscripten::function("shapes_binary_to_json", &shapes_binary_to_json);
}
#endif
