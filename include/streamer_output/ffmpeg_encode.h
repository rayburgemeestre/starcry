#pragma once

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
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
    int ret, x, y, got_output;
    FILE *f;
    AVFrame *frame;
    AVPacket pkt;

public:
    ffmpeg_h264_encode();
    void initialize(caf::event_based_actor *, int port);
    void add_frame(std::vector<ALLEGRO_COLOR> &pixels);
    void finalize();
};
