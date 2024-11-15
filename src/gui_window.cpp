/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gui_window.h"

#include "rendering_engine.h"
#include "util/logger.h"

#include <X11/Xutil.h>
#include <X11/keysym.h>

const int window_width = 640;
const int window_height = 480;

void requestRedrawSimple(Display* display, Window window) {
  XEvent event;
  memset(&event, 0, sizeof(event));
  event.type = Expose;
  event.xexpose.window = window;
  XSendEvent(display, window, False, ExposureMask, &event);
  XFlush(display);
}

// Helper function to create a scaled version of the source image
XImage* createScaledImage(Display* display, XImage* source, int dest_width, int dest_height) {
  XImage* scaled = XCreateImage(display,
                                DefaultVisual(display, DefaultScreen(display)),
                                source->depth,
                                ZPixmap,
                                0,
                                nullptr,
                                dest_width,
                                dest_height,
                                32,
                                0);

  scaled->data = (char*)malloc(dest_width * dest_height * 4);

  // Perform scaling
  for (int y = 0; y < dest_height; y++) {
    for (int x = 0; x < dest_width; x++) {
      // Map destination coordinates back to source coordinates
      float src_x = (float)x * source->width / dest_width;
      float src_y = (float)y * source->height / dest_height;

      // Simple nearest neighbor sampling
      int sx = (int)src_x;
      int sy = (int)src_y;

      // Clamp coordinates
      sx = std::min(sx, source->width - 1);
      sy = std::min(sy, source->height - 1);

      // Copy pixel
      unsigned long pixel = XGetPixel(source, sx, sy);
      XPutPixel(scaled, x, y, pixel);
    }
  }

  return scaled;
}

// Helper function to scale and draw the image
void drawImage(
    Display* display, Window window, GC gc, XImage* source, int window_width, int window_height, ScaleMode mode) {
  int src_width = source->width;
  int src_height = source->height;

  // Calculate destination dimensions based on scale mode
  int dest_x = 0, dest_y = 0;
  int dest_width = window_width;
  int dest_height = window_height;

  // Clear window with black
  XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
  XFillRectangle(display, window, gc, 0, 0, window_width, window_height);

  if (mode == ScaleMode::NONE) {
    // Center the image without scaling
    dest_width = src_width;
    dest_height = src_height;
    dest_x = (window_width - src_width) / 2;
    dest_y = (window_height - src_height) / 2;
    XPutImage(display, window, gc, source, 0, 0, dest_x, dest_y, dest_width, dest_height);
  } else if (mode == ScaleMode::STRETCH) {
    // Create and draw stretched image
    XImage* scaled = createScaledImage(display, source, window_width, window_height);
    XPutImage(display, window, gc, scaled, 0, 0, 0, 0, window_width, window_height);
    scaled->data = nullptr;  // Prevent double-free since XDestroyImage will free the data
    XDestroyImage(scaled);
  } else if (mode == ScaleMode::STRETCH_RATIO) {
    float src_ratio = static_cast<float>(src_width) / src_height;
    float window_ratio = static_cast<float>(window_width) / window_height;

    if (window_ratio > src_ratio) {
      // Window is wider than image ratio
      dest_width = static_cast<int>(window_height * src_ratio);
      dest_height = window_height;
      dest_x = (window_width - dest_width) / 2;
    } else {
      // Window is taller than image ratio
      dest_width = window_width;
      dest_height = static_cast<int>(window_width / src_ratio);
      dest_y = (window_height - dest_height) / 2;
    }

    // Create and draw aspect-ratio preserved scaled image
    XImage* scaled = createScaledImage(display, source, dest_width, dest_height);
    XPutImage(display, window, gc, scaled, 0, 0, dest_x, dest_y, dest_width, dest_height);
    scaled->data = nullptr;  // Prevent double-free
    XDestroyImage(scaled);
  }
}

gui_window::gui_window() : window_width_(window_width), window_height_(window_height) {
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
  display = XOpenDisplay(nullptr);
  if (!display) {
    return;
  }

  screen = DefaultScreen(display);
  root = RootWindow(display, screen);

  // Create the window
  window = XCreateSimpleWindow(display,
                               root,
                               100,
                               100,  // position
                               window_width,
                               window_height,                // size
                               1,                            // border width
                               BlackPixel(display, screen),  // border color
                               WhitePixel(display, screen)   // background color
  );

  // We want to get these events
  XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);

  // Set window size hints
  XSizeHints* size_hints = XAllocSizeHints();
  size_hints->flags = PMinSize;
  size_hints->min_width = 200;   // Minimum window width
  size_hints->min_height = 150;  // Minimum window height
  XSetWMNormalHints(display, window, size_hints);
  XFree(size_hints);

  // Make the window visible
  XMapWindow(display, window);

  // Create GC for drawing
  gc = XCreateGC(display, window, 0, nullptr);

  // Create an image to draw our gradient
  image = XCreateImage(display,
                       DefaultVisual(display, screen),
                       DefaultDepth(display, screen),
                       ZPixmap,
                       0,
                       nullptr,
                       window_width,
                       window_height,
                       32,
                       0);

  image->data = (char*)malloc(window_width * window_height * 4);  // 4 bytes per pixel

  current_width = window_width_;
  current_height = window_height_;

  runner = std::thread([&]() {
    runner_flag = true;
    while (runner_flag) {
      update_window();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteWindow, 1);
}

gui_window::~gui_window() {
  if (runner_flag) {
    if (runner.joinable()) runner.join();
  }

  stop();
}

void gui_window::stop() {
  free(image->data);
  image->data = nullptr;
  XDestroyImage(image);
  XFreeGC(display, gc);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}

void gui_window::add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<uint32_t>& pixels) {
  std::unique_lock lock(mut);
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

void gui_window::finalize() {
  runner_flag = false;
  runner.join();
  // window.close();
}

void gui_window::toggle_window() {
  std::unique_lock lock(mut);

  // TODO: not implemented
}

void requestRedraw(Display* display, Window window, GC gc, XImage* image, int width, int height, ScaleMode mode) {
  // Create off-screen buffer
  Pixmap buffer = XCreatePixmap(display, window, width, height, DefaultDepth(display, DefaultScreen(display)));

  // Draw to buffer first
  drawImage(display, buffer, gc, image, width, height, mode);

  // Copy buffer to window in one operation
  XCopyArea(display, buffer, window, gc, 0, 0, width, height, 0, 0);

  // Clean up
  XFreePixmap(display, buffer);
  XFlush(display);
}

void gui_window::update_window() {
  std::unique_lock lock(mut);
  if (!running) return;

  // Clear previous
  free(image->data);
  image->data = nullptr;
  XDestroyImage(image);

  // Create new
  image = XCreateImage(display,
                       DefaultVisual(display, screen),
                       DefaultDepth(display, screen),
                       ZPixmap,
                       0,
                       nullptr,
                       cached_canvas_w,
                       cached_canvas_h,
                       32,
                       0);

  image->data = (char*)malloc(cached_canvas_w * cached_canvas_h * 4);
  size_t index = 0;
  char bytes[4] = {0x00};
  for (int y = 0; y < cached_canvas_h; y++) {
    for (int x = 0; x < cached_canvas_w; x++) {
      char* byte_ptr = (char*)&(cached_pixels[index++]);
      bytes[0] = *byte_ptr++;
      bytes[1] = *byte_ptr++;
      bytes[2] = *byte_ptr++;
      bytes[3] = *byte_ptr++;
      XPutPixel(image, x, y, *(unsigned long*)&bytes);
    }
  }

  while (XPending(display)) {
    XEvent event;
    XNextEvent(display, &event);

    switch (event.type) {
      case Expose:
        drawImage(display, window, gc, image, current_width, current_height, scale_mode);
        break;

      case ConfigureNotify:
        // Window has been resized
        current_width = event.xconfigure.width;
        current_height = event.xconfigure.height;
        drawImage(display, window, gc, image, current_width, current_height, scale_mode);
        break;

      case KeyPress: {
        KeySym keysym = XLookupKeysym(&event.xkey, 0);
        if (keysym == XK_Escape) {
          running = false;
        } else if (keysym == XK_1) {
          scale_mode = ScaleMode::NONE;
          drawImage(display, window, gc, image, current_width, current_height, scale_mode);
        } else if (keysym == XK_2) {
          scale_mode = ScaleMode::STRETCH;
          drawImage(display, window, gc, image, current_width, current_height, scale_mode);
        } else if (keysym == XK_3) {
          scale_mode = ScaleMode::STRETCH_RATIO;
          drawImage(display, window, gc, image, current_width, current_height, scale_mode);
        }
        break;
      }
      case ClientMessage: {
        if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteWindow) {
          // Window close requested
          running = false;
        }
        break;
      }
    }
  }

  if (vsync) {
    requestRedraw(display, window, gc, image, current_width, current_height, scale_mode);
  } else {
    requestRedrawSimple(display, window);
  }
  if (!running) {
    stop();
  }
}
