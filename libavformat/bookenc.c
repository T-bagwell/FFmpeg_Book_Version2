/*
 * BOOK muxer
 */

#include <stdint.h>
#include <inttypes.h>

#include "avformat.h"
#include "internal.h"
#include "mux.h"
#include "libavutil/opt.h"

typedef struct BookMuxContext {
    AVClass *class;
    uint32_t magic;
    uint32_t version;
    uint32_t sample_rate;
    uint8_t channels;
    uint32_t width;
    uint32_t height;
    AVRational fps;
    char *info;
} BookMuxContext;

static const AVOption options[] = {
    { "info", "Book info", offsetof(BookMuxContext, info), AV_OPT_TYPE_STRING, {.str = NULL}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM, "bookflags" },
    { "version", "Book version", offsetof(BookMuxContext, version), AV_OPT_TYPE_INT, {.i64 = 1}, 0, 9, AV_OPT_FLAG_ENCODING_PARAM, "bookflags" },
    { NULL },
};

static const AVClass book_muxer_class = {
    .class_name = "book muxer",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

static AVStream *find_stream(AVFormatContext *s, enum AVMediaType type)
{
    int i = 0;
    for (i = 0; i < s->nb_streams; i++) {
        AVStream *stream = s->streams[i];
        if (stream->codecpar->codec_type == type) {
            return stream;
        }
    }
    return NULL;
}

static int book_init(AVFormatContext *s)
{
    AVStream *st;
    BookMuxContext *book = s->priv_data;
    printf("init nb_streams: %d\n", s->nb_streams);
    if (s->nb_streams < 2) {
        return AVERROR_INVALIDDATA;
    }
    st = find_stream(s, AVMEDIA_TYPE_AUDIO);
    if (!st) return AVERROR_INVALIDDATA;
    book->sample_rate = st->codecpar->sample_rate;
    book->channels = st->codecpar->channels;
    st = find_stream(s, AVMEDIA_TYPE_VIDEO);
    if (!st) return AVERROR_INVALIDDATA;
    book->width = st->codecpar->width;
    book->height = st->codecpar->height;
    // book->fps = st->codecpar->fps;
    book->fps = (AVRational){15, 1};
    return 0;
}

static int book_write_header(AVFormatContext *s)
{
    BookMuxContext *book = s->priv_data;
    book->magic = MKTAG('B', 'O', 'O', 'K');
    // avio_wb32(s->pb, book->magic);
    avio_write(s->pb, (uint8_t *)&book->magic, 4);
    avio_wb32(s->pb, book->version);
    avio_wb32(s->pb, book->sample_rate);
    avio_w8(s->pb, 0);
    avio_w8(s->pb, book->channels);
    avio_wb16(s->pb, book->width);
    avio_wb16(s->pb, book->height);
    avio_wb16(s->pb, book->fps.num);
    avio_wb16(s->pb, book->fps.den);
    char info[26] = {0};
    if (book->info) {
        strncpy(info, book->info, sizeof(info) - 1);
    }
    avio_write(s->pb, info, sizeof(info));
    return 0;
}

static int book_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    // BookMuxContext *mov = s->priv_data;
    uint32_t size = pkt->size;

    if (!pkt) {
        return 1;
    }

    AVStream *st = s->streams[pkt->stream_index];

    if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        printf("Audio: %04d pts: %lld\n", size, pkt->pts);
    } else if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        printf("Video: %04d pts: %lld\n", size, pkt->pts);
        size |= (1 << 31);
    } else {
        return 0; // ignore any other types
    }

    avio_wb64(s->pb, pkt->pts);
    avio_wb32(s->pb, size);
    avio_write(s->pb, pkt->data, pkt->size);

    return 0;
}

static int book_write_trailer(AVFormatContext *s)
{
    return 0;
}

static void book_free(AVFormatContext *s)
{
}

const FFOutputFormat ff_book_muxer = {
    .p.name              = "book",
    .p.long_name         = NULL_IF_CONFIG_SMALL("BOOK / BOOK"),
    .p.extensions        = "book",
    .priv_data_size    = sizeof(BookMuxContext),
    .p.audio_codec       = AV_CODEC_ID_AAC,
    .p.video_codec       = AV_CODEC_ID_BOOK,
    .init              = book_init,
    .write_header      = book_write_header,
    .write_packet      = book_write_packet,
    .write_trailer     = book_write_trailer,
    .deinit            = book_free,
    .p.flags             = 0,
    .p.priv_class        = &book_muxer_class,
};
