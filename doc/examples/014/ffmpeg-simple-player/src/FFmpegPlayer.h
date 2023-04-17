#ifndef FFMPEGPLAYER_H
#define FFMPEGPLAYER_H

#include "PacketQueue.h"

#ifdef __cplusplus
extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <SDL.h>
}
#endif

#include <string>
#include <functional>
#include <atomic>

#define MAX_AUDIO_FRAME_SIZE 192000

#define VIDEO_PICTURE_QUEUE_SIZE 1

#define FF_BASE_EVENT   (SDL_USEREVENT + 100)
#define FF_REFRESH_EVENT (FF_BASE_EVENT + 20)
#define FF_QUIT_EVENT    (FF_BASE_EVENT + 30)

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

#define SDL_AUDIO_BUFFER_SIZE (1024)

typedef void (*Image_Cb)(unsigned char* data, int w, int h, void *userdata);

struct VideoPicture {
    AVFrame  *bmp = nullptr;
    double pts = 0.0;
};

enum PauseState {
    UNPAUSE = 0,
    PAUSE = 1
};

struct FFmpegPlayerCtx {

    AVFormatContext *formatCtx = nullptr;

    AVCodecContext *aCodecCtx = nullptr;
    AVCodecContext *vCodecCtx = nullptr;

    int             videoStream = -1;
    int             audioStream = -1;

    AVStream        *audio_st = nullptr;
    AVStream        *video_st = nullptr;

    PacketQueue     audioq;
    PacketQueue     videoq;

    uint8_t         audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int    audio_buf_size = 0;
    unsigned int    audio_buf_index = 0;
    AVFrame         *audio_frame = nullptr;
    AVPacket        *audio_pkt = nullptr;
    uint8_t         *audio_pkt_data = nullptr;
    int             audio_pkt_size = 0;

    // seek flags and pos for seek
    std::atomic<int> seek_req;
    int              seek_flags;
    int64_t          seek_pos;

    // flush flag for seek
    std::atomic<bool> flush_actx = false;
    std::atomic<bool> flush_vctx = false;

    // for sync
    double          audio_clock = 0.0;
    double          frame_timer = 0.0;
    double          frame_last_pts = 0.0;
    double          frame_last_delay = 0.0;
    double          video_clock = 0.0;

    // picture queue
    VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
    std::atomic<int>             pictq_size = 0;
    std::atomic<int>             pictq_rindex = 0;
    std::atomic<int>             pictq_windex = 0;
    SDL_mutex       *pictq_mutex = nullptr;
    SDL_cond        *pictq_cond = nullptr;

    char            filename[1024];

    SwsContext      *sws_ctx = nullptr;
    SwrContext      *swr_ctx = nullptr;

    std::atomic<int> pause = UNPAUSE;

    // image callback
    Image_Cb        imgCb = nullptr;
    void            *cbData = nullptr;

    void init()
    {
        audio_frame = av_frame_alloc();
        audio_pkt = av_packet_alloc();

        pictq_mutex = SDL_CreateMutex();
        pictq_cond  = SDL_CreateCond();
    }

    void fini()
    {
        if (audio_frame) {
            av_frame_free(&audio_frame);
        }

        if (audio_pkt) {
            av_packet_free(&audio_pkt);
        }

        if (pictq_mutex) {
            SDL_DestroyMutex(pictq_mutex);
        }

        if (pictq_cond) {
            SDL_DestroyCond(pictq_cond);
        }
    }

};

class DemuxThread;
class VideoDecodeThread;
class AudioDecodeThread;
class AudioPlay;

class FFmpegPlayer
{
public:
    FFmpegPlayer();

    void setFilePath(const char *filePath);

    void setImageCb(Image_Cb cb, void *userData);

    int initPlayer();

    void start();

    void stop();

    void pause(PauseState state);

public:
    void onRefreshEvent(SDL_Event *e);
    void onKeyEvent(SDL_Event *e);

private:
    FFmpegPlayerCtx playerCtx;
    std::string m_filePath;
    SDL_AudioSpec audio_wanted_spec;
    std::atomic<bool> m_stop = false;

private:
    DemuxThread *m_demuxThread = nullptr;
    VideoDecodeThread *m_videoDecodeThread = nullptr;
    AudioDecodeThread *m_audioDecodeThread = nullptr;
    AudioPlay *m_audioPlay = nullptr;
};

#endif // FFMPEGPLAYER_H
