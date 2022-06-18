/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <fstream>
#include <stdexcept>
#include <string>

#include "data/bounding_box.hpp"
#include "data/shape.hpp"
#include "image.hpp"

#include "stb_truetype.h"

class text_drawer {
private:
  long size = 0;
  unsigned char *fontBuffer = nullptr;
  unsigned char *bitmap_ = nullptr;
  stbtt_fontinfo info;
  int bitmap_width_ = 1920;
  int bitmap_height_ = 32;
  int line_height = 32;

  void load_font() {
    std::string filename = "monaco.ttf";
    const auto exists = [](const std::string &filename) {
      std::ifstream infile(filename);
      return infile.good();
    };
    if (!exists(filename)) {
      filename = "/workdir/monaco.ttf";
    }
    if (!exists(filename)) {
      throw std::runtime_error("could not find font");
    }

    FILE *fontFile = fopen(filename.c_str(), "rb");
    fseek(fontFile, 0, SEEK_END);
    size = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);
    fontBuffer = (unsigned char *)malloc(size);
    fread(fontBuffer, size, 1, fontFile);
    fclose(fontFile);
  }

  void prepare_font() {
    if (!stbtt_InitFont(&info, fontBuffer, 0)) {
      throw std::runtime_error("prepare_font() failed.");
    }
  }

  void allocate_bitmap() {
    bitmap_ = (unsigned char *)calloc(bitmap_width_ * bitmap_height_, sizeof(unsigned char));
  }

public:
  text_drawer() {}

  text_drawer(int text_height) : bitmap_height_(text_height * 2 /* for rounding issues */), line_height(text_height) {
    load_font();
    prepare_font();
    allocate_bitmap();
  }

  ~text_drawer() {
    free(fontBuffer);
    free(bitmap_);
  }

  void draw(image &bmp, double target_x, double target_y, const std::string &text) {
    memset(bitmap_, 0x00, bitmap_width_ * bitmap_height_ * sizeof(unsigned char));
    float scale = stbtt_ScaleForPixelHeight(&info, line_height);
    const char *word = text.c_str();
    int x = 0;
    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    ascent = static_cast<int>(roundf(ascent * scale));
    descent = roundf(descent * scale);

    for (size_t i = 0; i < strlen(word); ++i) {
      // how wide is this character
      int ax = 0;
      int lsb = 0;
      stbtt_GetCodepointHMetrics(&info, word[i], &ax, &lsb);
      // get bounding box for character
      int c_x1 = 0, c_y1 = 0, c_x2 = 0, c_y2 = 0;
      stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
      // compute y (different characters have different heights
      int y = ascent + c_y1;
      // render character (stride and offset is important here)
      int byteOffset = x + roundf(lsb * scale) + (y * bitmap_width_);
      stbtt_MakeCodepointBitmap(
          &info, bitmap_ + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmap_width_, scale, scale, word[i]);
      // advance x
      x += roundf(ax * scale);
      // add kerning
      int kern = 0;
      kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
      x += roundf(kern * scale);
    }
  }

  bounding_box box() {
    bounding_box the_box;
    auto bitmap_pixel = bitmap_;
    for (int bitmap_y = 0; bitmap_y < bitmap_height_; bitmap_y++) {
      for (int bitmap_x = 0; bitmap_x < bitmap_width_; bitmap_x++) {
        double c = double(*(bitmap_pixel++)) / 255.;
        if (c) {
          the_box.update_x(bitmap_x);
          the_box.update_y(bitmap_y);
        }
      }
    }
    return the_box;
  }

  unsigned char *bitmap() {
    return bitmap_;
  };
  int bitmap_width() {
    return bitmap_width_;
  };
  int bitmap_height() {
    return bitmap_height_;
  };
};
