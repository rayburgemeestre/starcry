#pragma once

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#include <vector>
#include "allegro5/color.h"

class AbstractTimer;

class ffmpeg_h264_encode
{
private:
    std::string filename = "test.h264";
    AVCodecID codec_id = AV_CODEC_ID_H264;
    AVCodec *codec;
    AVCodecContext *c= NULL;
    size_t frameNumber = 0;
    int ret, x, y, got_output;
    FILE *f;
    AVFrame *frame;
    AVPacket pkt;
//    std::unique_ptr<AbstractTimer> timer;

public:
    ffmpeg_h264_encode();
    void initialize();
    void add_frame(std::vector<ALLEGRO_COLOR> &pixels);
    void finalize();
};
