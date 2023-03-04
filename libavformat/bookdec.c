/*
 * Book demuxer
 */

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include "libavcodec/avcodec.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "libavutil/pixfmt.h"
#include "avformat.h"
#include "demux.h"
#include "internal.h"

typedef struct BookContext {
    AVFormatContext *fc;
} BookContext;

typedef struct BookFormat {
    char magic[4];
    uint32_t version;
    uint32_t sample_rate;
    uint8_t channels;
    uint32_t width;
    uint32_t height;
    AVRational fps;
    char info[26];
} BookFormat;

static int book_probe(const AVProbeData *p)
{
    int score = AVPROBE_SCORE_MAX;
    uint32_t magic = AV_RN32(p->buf);

    printf("probe ... score=%d\n", score);

    if (magic != MKTAG('B', 'O', 'O', 'K')) {
        return AVERROR_INVALIDDATA;
    }

    return score;
}

static int book_read_header(AVFormatContext *s)
{
    AVStream *st;
    FFStream *sti;
    BookFormat fmt = { };
    uint8_t data[48];
    uint8_t *pdata = data;
    struct AVCodecContext *avctx = NULL;

    printf("reading header ...\n");
    avio_read(s->pb, data, 48);
    pdata += 4; fmt.version = AV_RB32(pdata);
    pdata += 4; fmt.sample_rate = AV_RB32(pdata);
    pdata += 5; fmt.channels = AV_RB8(pdata);
    pdata += 1; fmt.width = AV_RB16(pdata);
    pdata += 2; fmt.height = AV_RB16(pdata);
    pdata += 2; fmt.fps.num = AV_RB16(pdata);
    pdata += 2; fmt.fps.den = AV_RB16(pdata);
    pdata += 2; strncpy(fmt.info, pdata, sizeof(fmt.info) - 1);

    printf("version=%d\n", fmt.version);
    printf("sample_rate=%d\n", fmt.sample_rate);
    printf("width=%d\n", fmt.width);
    printf("height=%d\n", fmt.height);
    printf("fps=%d/%d\n", fmt.fps.num, fmt.fps.den);
    printf("info=%s\n", fmt.info);

    st = avformat_new_stream(s, NULL);
    if (!st) return AVERROR(ENOMEM);
    st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codecpar->codec_id = AV_CODEC_ID_AAC;
    st->codecpar->sample_rate = fmt.sample_rate;
    st->codecpar->channels = fmt.channels;
    sti = ffstream(st);
    sti->need_parsing = AVSTREAM_PARSE_NONE;
    // sti->need_parsing = AVSTREAM_PARSE_FULL_RAW;
    st->start_time = 0;

    st = avformat_new_stream(s, NULL);
    if (!st) return AVERROR(ENOMEM);
    st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    st->codecpar->codec_id = AV_CODEC_ID_H264;
    // st->codecpar->codec_id = AV_CODEC_ID_BOOK;
    st->codecpar->width = fmt.width;
    st->codecpar->height = fmt.height;
    st->codecpar->format = AV_PIX_FMT_YUV420P;
    st->start_time = 0;
    sti = ffstream(st);
    sti->need_parsing = AVSTREAM_PARSE_NONE;
    // sti->need_parsing = AVSTREAM_PARSE_FULL_RAW;
    avctx = ffstream(st)->avctx;
    avctx->codec_id = AV_CODEC_ID_H264;
    avctx->framerate = fmt.fps;
    avpriv_set_pts_info(st, 64, fmt.fps.den, fmt.fps.num);
    printf("done read header ... nb_streams=%d\n", s->nb_streams);

    return 0;
}

static int book_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    int ret;
    int stream_index = 0;
    uint64_t pts = avio_rb64(s->pb);
    uint32_t size = avio_rb32(s->pb);

    if (size & (1 << 31)) { // video
        size &= ~(1 << 31);
        stream_index = 1;
    }
    if ((ret = av_new_packet(pkt, size)) < 0) return ret;
    pkt->stream_index = stream_index;
    pkt->pos = avio_tell(s->pb);
    pkt->pts = pts;
    ret = avio_read(s->pb, pkt->data, size);
    if (ret < 0) {
        av_packet_unref(pkt);
        return ret;
    }
    if (ret < size) {
        av_packet_unref(pkt);
        return AVERROR_INVALIDDATA;
    }
    printf("read %s size: %04d pts: %llu\n", stream_index == 1 ? "video" : "audio", size, pkt->pts);

    return ret;
}

static int book_read_close(AVFormatContext *s)
{
    // BookContext *book = s->priv_data;
    return 0;
}

static const AVOption book_options[] = {
    { NULL },
};

static const AVClass book_class = {
    .class_name = "book",
    .item_name  = av_default_item_name,
    .option     = book_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

const AVInputFormat ff_book_demuxer = {
    .name           = "book",
    .long_name      = NULL_IF_CONFIG_SMALL("BOOK / BOOK"),
    .priv_class     = &book_class,
    .priv_data_size = sizeof(BookContext),
    .extensions     = "book",
    .flags_internal = FF_FMT_INIT_CLEANUP,
    .read_probe     = book_probe,
    .read_header    = book_read_header,
    .read_packet    = book_read_packet,
    .read_close     = book_read_close,
    .flags          = AVFMT_NO_BYTE_SEEK,
};
