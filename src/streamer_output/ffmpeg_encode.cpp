#include <iostream>

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

//extern void sfml_init(unsigned char **pixels, size_t *pixelSize);
//extern void sfml_generate_frame();

/**
	* Copy pixels to H264 frame pixels.
	*
	* @param pixels	raw pointer to all RGBA values from SFML image
	* @param c			struct that contains width and height for video
	* @param frame		video struct that contains pixels in YUV / YCrCb.
	*/
void transfer_pixels(std::vector<ALLEGRO_COLOR> &pixels, AVCodecContext * c, AVFrame * frame)
{
	/* Y, Cb and Cr */
	size_t index = 0;
	for(int y=0;y<c->height;y++) {
		for(int x=0;x<c->width;x++) {
			ALLEGRO_COLOR &currentPixel(pixels[index++]);
			float R = currentPixel.r * 255;
			float G = currentPixel.g * 255;
			float B = currentPixel.b * 255;

			float Y = (0.257 * R) + (0.504 * G) + (0.098 * B) + 16;
			float Cb = (-0.148 * R) - (0.291 * G) + (0.439 * B) + 128;
			float Cr = (0.439 * R) - (0.368 * G) - (0.071 * B) + 128;

			frame->data[0][y * frame->linesize[0] + x] = Y;
			if ((y % 2) == 0 && (x % 2) == 0) {
				frame->data[1][(y/2) * frame->linesize[1] + (x/2)] = Cb;
				frame->data[2][(y/2) * frame->linesize[2] + (x/2)] = Cr;
			}
		}
	}
}

ffmpeg_h264_encode::ffmpeg_h264_encode() {}

void ffmpeg_h264_encode::initialize(uint32_t canvas_w, uint32_t canvas_h, caf::event_based_actor *, int) {
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
    c->bit_rate = 400000 * 10;
    /* resolution must be a multiple of two */
    c->width = canvas_w;
    c->height = canvas_h;

    /* frames per second (we'll recode later anyway)*/
    c->time_base.num = 1;
    c->time_base.den = 25;

    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames=1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if(codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);

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
    frame->width  = c->width;
    frame->height = c->height;

    /* the image can be allocated by any means and av_image_alloc() is
        * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
                         c->pix_fmt, 32);
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

void ffmpeg_h264_encode::add_frame(uint32_t canvas_w, uint32_t canvas_h, std::vector<ALLEGRO_COLOR> &pixels) {

    transfer_pixels(pixels, c, frame);

    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    fflush(stdout);

    frame->pts = frameNumber;
    // Will replace the pts calculation with something better
    //frame->pts += av_rescale_q(1, c->time_base, c->time_base);

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(1);
    }

    if (got_output) {
        //printf("Write frame %3d (size=%5d)\n", frameNumber, pkt.size);
        fwrite(pkt.data, 1, pkt.size, f);
        av_free_packet(&pkt);
    }
    frameNumber++;
}

void ffmpeg_h264_encode::finalize()
{
    /* get the delayed frames */
    for (got_output = 1; got_output; frameNumber++) {
        fflush(stdout);

        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output) {
            //printf("Write frame %3d (size=%5d)\n", frameNumber, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    /* add sequence end code to have a real mpeg file */
    // does not seem to work for H264
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);

    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    printf("\n");
}

