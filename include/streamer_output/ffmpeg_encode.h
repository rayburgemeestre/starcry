/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h> // for streaming
#include <libavutil/imgutils.h>
}

#include <vector>
#include <mutex>
#include "allegro5/color.h"

class AbstractTimer;
namespace caf {
    class event_based_actor;
}

class ffmpeg_h264_encode
{
private:
    std::string filename = "test.h264";
    AVCodecID codec_id = AV_CODEC_ID_H264;
    AVCodec *codec;
    AVCodecContext *c= NULL;
    size_t frameNumber = 0;
    size_t fps_ = 25;
    int ret, got_output;
    FILE *f;
    AVFrame *frame;
    AVPacket pkt;

public:
    ffmpeg_h264_encode(std::string filename, size_t bitrate, size_t fps, uint32_t canvas_w, uint32_t canvas_h);
    void add_frame(std::vector<uint32_t> &pixels);
    void finalize();
};
