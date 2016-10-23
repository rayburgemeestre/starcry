#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern "C" {
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <string>
#include <vector>
#include "benchmark.h"

class ffmpeg_flv_stream
{
private:
    std::string url_ = "rtmp://localhost/flvplayback/video";
    uint32_t width_;
    uint32_t height_;

    // some properties
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVStream *audio_st, *video_st;
    AVCodec *audio_codec, *video_codec;
    double audio_pts, video_pts;
    int ret;

    bool audio = false;

    AVFrame *frame;
    AVPicture src_picture, dst_picture;
    int frame_count;

    std::unique_ptr<AbstractTimer> timer;
    int64_t currentframe = 0;
    size_t fps_;


public:
    ffmpeg_flv_stream(std::string url, size_t bitrate, size_t fps, uint32_t canvas_w, uint32_t canvas_h);
    void add_frame(std::vector<uint32_t> &pixels);
    void finalize();

protected:
    AVStream *add_stream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
    void open_video(AVFormatContext *oc, AVCodec *codec, AVStream *st);
    void open_audio(AVFormatContext *oc, AVCodec *codec, AVStream *st);
    void write_video_frame(std::vector<uint32_t> &pixels, AVFormatContext *oc, AVStream *st);
    void close_video(AVFormatContext *oc, AVStream *st);
};
