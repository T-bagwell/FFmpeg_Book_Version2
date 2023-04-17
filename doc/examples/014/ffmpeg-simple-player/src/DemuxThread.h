#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H

#include "ThreadBase.h"

#include <string>

struct FFmpegPlayerCtx;

class DemuxThread : public ThreadBase
{
public:
    DemuxThread();

    void setPlayerCtx(FFmpegPlayerCtx *ctx);

    int initDemuxThread();

    void finiDemuxThread();

    void run();

private:
    int decode_loop();
    int audio_decode_frame(FFmpegPlayerCtx *is, double *pts_ptr);
    int stream_open(FFmpegPlayerCtx *is, int media_type);

private:
    FFmpegPlayerCtx *is = nullptr;
};

#endif // DEMUXTHREAD_H
