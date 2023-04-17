#include "DemuxThread.h"

#include <functional>

#include "log.h"
#include "FFmpegPlayer.h"

DemuxThread::DemuxThread()
{

}

void DemuxThread::setPlayerCtx(FFmpegPlayerCtx *ctx)
{
    is = ctx;
}

int DemuxThread::initDemuxThread()
{
    AVFormatContext *formatCtx = NULL;
    if (avformat_open_input(&formatCtx, is->filename, NULL, NULL) != 0) {
        ff_log_line("avformat_open_input Failed.");
        return -1;
    }

    is->formatCtx = formatCtx;

    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        ff_log_line("avformat_find_stream_info Failed.");
        return -1;
    }

    av_dump_format(formatCtx, 0, is->filename, 0);

    if (stream_open(is, AVMEDIA_TYPE_AUDIO) < 0) {
        ff_log_line("open audio stream Failed.");
        return -1;
    }

    if (stream_open(is, AVMEDIA_TYPE_VIDEO) < 0) {
        ff_log_line("open video stream Failed.");
        return -1;
    }

    return 0;
}

void DemuxThread::finiDemuxThread()
{
    if (is->formatCtx) {
        avformat_close_input(&is->formatCtx);
        is->formatCtx = nullptr;
    }

    if (is->aCodecCtx) {
        avcodec_free_context(&is->aCodecCtx);
        is->aCodecCtx = nullptr;
    }

    if (is->vCodecCtx) {
        avcodec_free_context(&is->vCodecCtx);
        is->vCodecCtx = nullptr;
    }

    if (is->swr_ctx) {
        swr_free(&is->swr_ctx);
        is->swr_ctx = nullptr;
    }

    if (is->sws_ctx) {
        sws_freeContext(is->sws_ctx);
        is->sws_ctx = nullptr;
    }
}

void DemuxThread::run()
{
    decode_loop();
}

int DemuxThread::decode_loop()
{
    AVPacket *packet = av_packet_alloc();

    for(;;) {
        if(m_stop) {
            ff_log_line("request quit while decode_loop");
            break;
        }

        // begin seek
        if (is->seek_req) {
            int stream_index= -1;
            int64_t seek_target = is->seek_pos;

            if (is->videoStream >= 0) {
                stream_index = is->videoStream;
            } else if(is->audioStream >= 0) {
                stream_index = is->audioStream;
            }

            if (stream_index >= 0) {
                seek_target= av_rescale_q(seek_target, AVRational{1, AV_TIME_BASE}, is->formatCtx->streams[stream_index]->time_base);
            }

            if (av_seek_frame(is->formatCtx, stream_index, seek_target, is->seek_flags) < 0) {
                ff_log_line("%s: error while seeking\n", is->filename);
            } else {
                if(is->audioStream >= 0) {
                    is->audioq.packetFlush();
                    is->flush_actx = true;
                }
                if (is->videoStream >= 0) {
                    is->videoq.packetFlush();
                    is->flush_vctx = true;
                }
            }

            // reset to zero when seeking done
            is->seek_req = 0;
        }

        if (is->audioq.packetSize() > MAX_AUDIOQ_SIZE || is->videoq.packetSize() > MAX_VIDEOQ_SIZE) {
            SDL_Delay(10);
            continue;
        }

        if (av_read_frame(is->formatCtx, packet) < 0) {
            ff_log_line("av_read_frame error");
            break;
        }

        if (packet->stream_index == is->videoStream) {
            is->videoq.packetPut(packet);
        } else if (packet->stream_index == is->audioStream) {
            is->audioq.packetPut(packet);
        } else {
            av_packet_unref(packet);
        }
    }

    while (!m_stop) {
        SDL_Delay(100);
    }

    av_packet_free(&packet);

    SDL_Event event;
    event.type = FF_QUIT_EVENT;
    event.user.data1 = is;
    SDL_PushEvent(&event);

    return 0;
}

int DemuxThread::stream_open(FFmpegPlayerCtx *is, int media_type)
{
    AVFormatContext *formatCtx = is->formatCtx;
    AVCodecContext *codecCtx = NULL;
    AVCodec *codec = NULL;

    int stream_index = av_find_best_stream(formatCtx, (AVMediaType)media_type, -1, -1, (const AVCodec **)&codec, 0);
    if (stream_index < 0 || stream_index >= (int)formatCtx->nb_streams) {
        ff_log_line("Cannot find a audio stream in the input file\n");
        return -1;
    }

    codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, formatCtx->streams[stream_index]->codecpar);

    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        ff_log_line("Failed to open codec for stream #%d\n", stream_index);
        return -1;
    }

    switch(codecCtx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->audioStream = stream_index;
        is->aCodecCtx = codecCtx;
        is->audio_st = formatCtx->streams[stream_index];
        is->swr_ctx = swr_alloc();

        av_opt_set_chlayout(is->swr_ctx, "in_chlayout", &codecCtx->ch_layout, 0);
        av_opt_set_int(is->swr_ctx, "in_sample_rate",       codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(is->swr_ctx, "in_sample_fmt", codecCtx->sample_fmt, 0);

        AVChannelLayout outLayout;
        // use stereo
        av_channel_layout_default(&outLayout, 2);

        av_opt_set_chlayout(is->swr_ctx, "out_chlayout", &outLayout, 0);
        av_opt_set_int(is->swr_ctx, "out_sample_rate",       48000, 0);
        av_opt_set_sample_fmt(is->swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        swr_init(is->swr_ctx);

        break;
    case AVMEDIA_TYPE_VIDEO:
        is->videoStream = stream_index;
        is->vCodecCtx   = codecCtx;
        is->video_st    = formatCtx->streams[stream_index];
        is->frame_timer = (double)av_gettime() / 1000000.0;
        is->frame_last_delay = 40e-3;
        is->sws_ctx = sws_getContext(
                    codecCtx->width,
                    codecCtx->height,
                    codecCtx->pix_fmt,
                    codecCtx->width,
                    codecCtx->height,
                    AV_PIX_FMT_RGB24,
                    SWS_BILINEAR,
                    NULL,
                    NULL,
                    NULL
                    );
        break;
    default:
        break;
    }

    return 0;
}
