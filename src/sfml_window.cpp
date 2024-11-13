/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sfml_window.h"
#include <stdexcept>
#include "rendering_engine.h"
#include "util/logger.h"

const int window_width = 640;
const int window_height = 480;

sfml_window::sfml_window() : window(sf::VideoMode(window_width, window_height, bpp), "Preview") {
  cached_canvas_w = window_width;
  cached_canvas_h = window_height;
  for (size_t y = 0; y < window_height; y++) {
    for (size_t x = 0; x < window_width; x++) {
      cached_pixels.emplace_back(std::clamp((double)x, 0., 1.));
      cached_pixels.emplace_back(std::clamp((double)y, 0., 1.));
      cached_pixels.emplace_back(std::clamp((double)x * (double)y, 0., 1.));
      cached_pixels.emplace_back(std::clamp((double)x / (double)y, 0., 1.));
    }
  }
  if (vsync) window.setVerticalSyncEnabled(true);
  window.setActive(false);
  sf::Clock clock, total_clock;
  runner = std::thread([&]() {
    runner_flag = true;
    while (runner_flag) {
      update_window();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });
}

sfml_window::~sfml_window() {
  if (runner_flag) {
    if (runner.joinable()) runner.join();
  }
}

void sfml_window::add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels) {
  std::unique_lock<std::mutex> lock(mut);
  canvas_h = pixels.size() / canvas_w;
  cached_canvas_w = canvas_w;
  cached_canvas_h = canvas_h;
  cached_pixels = pixels;
  if (pixels.size() < canvas_w * canvas_h) {
    cached_canvas_w = 0;
    cached_canvas_h = 0;
    return;
  }
  lock.unlock();
  update_window();
}

void sfml_window::finalize() {
  runner_flag = false;
  runner.join();
  window.close();
}

void sfml_window::toggle_window() {
  std::unique_lock<std::mutex> lock(mut);
  if (window.isOpen()) {
    window.close();
  } else {
    window.create(sf::VideoMode(window_width, window_height, bpp), "Preview");
  }
}

void sfml_window::update_window() {
  std::unique_lock lock(mut);
  if (!window.isOpen()) return;
  sf::Context context;
  sf::View view = window.getDefaultView();
  view.setCenter({static_cast<float>(cached_canvas_w / 2.0), static_cast<float>(cached_canvas_h / 2.0)});
  view.setSize({static_cast<float>(cached_canvas_w), static_cast<float>(cached_canvas_h)});
  if (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if ((event.type == sf::Event::Closed) ||
          ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))) {
        window.close();
        return;
      }
      if (event.type == sf::Event::Resized) {
        // resize is implicitly by our thread that executes every 100 milliseconds
        // and will update the view based on the window size.
      }
    }
    window.setView(view);
    if (sf::Vector2u windowSize = window.getSize(); !(windowSize.x >= window_width && windowSize.y >= window_height)) {
      return;
    }
    sf::Texture texture;
    sf::Image image;
    if (cached_canvas_w && cached_canvas_h) {
      image.create(cached_canvas_w, cached_canvas_h, (const sf::Uint8 *)(&(cached_pixels[0])));
      texture.loadFromImage(image);
      sf::Sprite sprite(texture);
      window.draw(sprite);
    }
    window.display();
  }
}
