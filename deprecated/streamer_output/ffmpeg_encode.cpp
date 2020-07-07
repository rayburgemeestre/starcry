/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
/**
 * @file
 * libavcodec API use example.
 *
 * Note that libavcodec only handles codecs (mpeg, mpeg4, etc...),
 * not file formats (avi, vob, mp4, mov, mkv, mxf, flv, mpegts, mpegps, etc...). See library 'libavformat' for the
 * format handling
 * @example doc/examples/decoding_encoding.c
 */

// RBU add avutil.lib, avcodec.lib

#include <math.h>

#include "streamer_output/ffmpeg_encode.h"
//#include "benchmark.h"

// extern void sfml_init(unsigned char **pixels, size_t *pixelSize);
// extern void sfml_generate_frame();

/**
 * Copy pixels to H264 frame pixels.
 *
 * @param pixels	raw pointer to all RGBA values from SFML image
 * @param c			struct that contains width and height for video
 * @param frame		video struct that contains pixels in YUV / YCrCb.
 */

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_pixels.h"
template <typename T>
inline void transfer_pixels(std::vector<uint32_t> &pixels, AVCodecContext *c, T *frame) {
  ALLEGRO_COLOR color{0};
  /* Y, Cb and Cr */
  size_t index = 0;
  for (int y = 0; y < c->height; y++) {
    for (int x = 0; x < c->width; x++) {
      uint32_t &_gp_pixel(pixels[index++]);
      _AL_MAP_RGBA(color,
                   (_gp_pixel & 0x00FF0000) >> 16,
                   (_gp_pixel & 0x0000FF00) >> 8,
                   (_gp_pixel & 0x000000FF) >> 0,
                   (_gp_pixel & 0xFF000000) >> 24);
      uint8_t R = static_cast<uint8_t>(color.r * 255);
      uint8_t G = static_cast<uint8_t>(color.g * 255);
      uint8_t B = static_cast<uint8_t>(color.b * 255);

      // TODO: This RGBA -> CMYK conversion makes stuff considerably slower, perhaps investigate
      // how to use RGBA directly with the ffmpeg video..
      uint8_t Y = static_cast<uint8_t>((0.257 * R) + (0.504 * G) + (0.098 * B) + 16);
      uint8_t Cb = static_cast<uint8_t>((-0.148 * R) - (0.291 * G) + (0.439 * B) + 128);
      uint8_t Cr = static_cast<uint8_t>((0.439 * R) - (0.368 * G) - (0.071 * B) + 128);

      frame->data[0][y * frame->linesize[0] + x] = Y;
      if ((y % 2) == 0 && (x % 2) == 0) {
        frame->data[1][(y / 2) * frame->linesize[1] + (x / 2)] = Cb;
        frame->data[2][(y / 2) * frame->linesize[2] + (x / 2)] = Cr;
      }
    }
  }
}
void transfer_pixels(std::vector<uint32_t> &pixels, AVCodecContext *c, AVFrame *frame);    // used by this file
void transfer_pixels(std::vector<uint32_t> &pixels, AVCodecContext *c, AVPicture *frame);  // used by stream version

void transfer_pixels_avframe(std::vector<uint32_t> &pixels, AVCodecContext *c, AVFrame *frame) {
  transfer_pixels<AVFrame>(pixels, c, frame);
}
void transfer_pixels_avpicture(std::vector<uint32_t> &pixels, AVCodecContext *c, AVPicture *frame) {
  transfer_pixels<AVPicture>(pixels, c, frame);
}

ffmpeg_h264_encode::ffmpeg_h264_encode(
    std::string filename, size_t bitrate, size_t fps, uint32_t canvas_w, uint32_t canvas_h) {
  fps_ = fps;

  avcodec_register_all();

  printf("Encode video file %s\n", filename.c_str());
  /* find the mpeg1 video encoder */
  codec = avcodec_find_encoder(codec_id);
  if (!codec) {
    fprintf(stderr, "Codec not found\n");
    exit(1);
  }

  c = avcodec_alloc_context3(codec);
  if (!c) {
    fprintf(stderr, "Could not allocate video codec context\n");
    exit(1);
  }

  /* put sample parameters */
  // c->bit_rate = 400000 * 10;
  std::cout << "using bitrate: " << bitrate << std::endl;
  c->bit_rate = bitrate;
  /* resolution must be a multiple of two */
  c->width = canvas_w;
  c->height = canvas_h;

  /* frames per second (we'll recode later anyway)*/
  c->time_base.num = 1;
  c->time_base.den = static_cast<int>(fps_);

  c->gop_size = 10; /* emit one intra frame every ten frames */
  c->max_b_frames = 1;
  c->pix_fmt = AV_PIX_FMT_YUV420P;

  if (codec_id == AV_CODEC_ID_H264) av_opt_set(c->priv_data, "preset", "slow", 0);

  /* open it */
  if (avcodec_open2(c, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }

  f = fopen(filename.c_str(), "wb");
  if (!f) {
    fprintf(stderr, "Could not open %s\n", filename.c_str());
    exit(1);
  }

  frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "Could not allocate video frame\n");
    exit(1);
  }
  frame->format = c->pix_fmt;
  frame->width = c->width;
  frame->height = c->height;

  /* the image can be allocated by any means and av_image_alloc() is
   * just the most convenient way if av_malloc() is to be used */
  ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
  if (ret < 0) {
    fprintf(stderr, "Could not allocate raw picture buffer\n");
    exit(1);
  }

  frame->pts = 0;
  //#ifdef WIN32
  //    timer = TimerFactory::factory(TimerFactory::Type::WindowsHRTimerImpl);
  //#else
  //    timer = TimerFactory::factory(TimerFactory::Type::BoostTimerImpl);
  //#endif
  //    timer->start();
}

void ffmpeg_h264_encode::add_frame(std::vector<uint32_t> &pixels) {
  transfer_pixels_avframe(pixels, c, frame);

  av_init_packet(&pkt);
  pkt.data = NULL;  // packet data will be allocated by the encoder
  pkt.size = 0;

  fflush(stdout);

  frame->pts = frameNumber;
// Will replace the pts calculation with something better
// frame->pts += av_rescale_q(1, c->time_base, c->time_base);

/* encode the image */
// deprecated perhaps, but the documentation still uses it in the examples
// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decoding_encoding.c
// I cannot find a proper example for the new non-deprecated API
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  if (ret < 0) {
    fprintf(stderr, "Error encoding frame\n");
    exit(1);
  }

  if (got_output) {
    // printf("Write frame %3d (size=%5d)\n", frameNumber, pkt.size);
    fwrite(pkt.data, 1, pkt.size, f);
    av_packet_unref(&pkt);
  }
  frameNumber++;
}

void ffmpeg_h264_encode::finalize() {
  /* get the delayed frames */
  for (got_output = 1; got_output; frameNumber++) {
    fflush(stdout);

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
    if (ret < 0) {
      fprintf(stderr, "Error encoding frame\n");
      exit(1);
    }

    if (got_output) {
      // printf("Write frame %3d (size=%5d)\n", frameNumber, pkt.size);
      fwrite(pkt.data, 1, pkt.size, f);
      av_packet_unref(&pkt);
    }
  }

  /* add sequence end code to have a real mpeg file */
  // does not seem to work for H264
  uint8_t endcode[] = {0, 0, 1, 0xb7};
  fwrite(endcode, 1, sizeof(endcode), f);
  fclose(f);

  avcodec_close(c);
  av_free(c);
  av_freep(&frame->data[0]);
  av_frame_free(&frame);
  printf("\n");
}
#pragma GCC diagnostic pop