/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "streamer_output/sfml_window.h"
#include <rendering_engine_wrapper.h>
#include <stdexcept>

const int window_width = 100;
const int window_height = 100;

sfml_window::sfml_window()
    : window(sf::VideoMode(window_width, window_height, bpp), "Bouncing ball"), ball(ball_radius - 4) {
  if (vsync) window.setVerticalSyncEnabled(true);

  window.setActive(false);

  //  font.loadFromFile("monaco.ttf");
  //  text.setFont(font); // font is a sf::Font
  //  text.setCharacterSize(24); // in pixels, not points!
  //  text.setFillColor(sf::Color::Red);
  //  text.setStyle(sf::Text::Bold | sf::Text::Underlined);

  sf::Clock clock, total_clock;
  sf::Time elapsed = clock.restart();
}

void sfml_window::add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t> &pixels) {
  // ARGB -> ABGR
  for (auto &pixel : pixels) {
    auto *R = (uint8_t *)&pixel;
    auto *B = R + 2;
    std::swap(*R, *B);
  }

  sf::Context context;

  sf::View view = window.getDefaultView();
  view.setCenter({static_cast<float>(canvas_w / 2.0), static_cast<float>(canvas_h / 2.0)});
  view.setSize({static_cast<float>(canvas_w), static_cast<float>(canvas_h)});
  window.setView(view);

  if (window.isOpen()) {
    sf::Event event;
    if (window.pollEvent(event)) {
      if ((event.type == sf::Event::Closed) ||
          ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))) {
        window.close();
        return;
      }
      if (event.type == sf::Event::Resized) {
      }
    }
    // text.setString("Hello world");
    window.clear(sf::Color(30, 30, 120));
    // window.draw(text);

    sf::Vector2u windowSize = window.getSize();
    if (!(windowSize.x >= window_width && windowSize.y >= window_height)) {
      return;
    }

    sf::Texture texture;
    sf::Image screenshot;
    screenshot.create(canvas_w, canvas_h, (const sf::Uint8 *)(&(pixels[0])));
    texture.loadFromImage(screenshot);
    sf::Sprite sprite(texture);
    window.draw(sprite);
    window.display();
  }
}

void sfml_window::finalize() {
  window.close();
}
