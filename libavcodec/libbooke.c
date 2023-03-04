#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "avcodec.h"
#include "codec_internal.h"
#include "encode.h"
#include "internal.h"

typedef struct libbookContext {
    const AVClass *class;
    AVFrame *frame;
} libbookContext;

static av_cold int libbook_encode_init(AVCodecContext *avctx)
{
    libbookContext *ctx = avctx->priv_data;
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(avctx->pix_fmt);
    av_log(avctx, AV_LOG_INFO, "libbook pix_fmt: %s\n", desc->name);
    if (avctx->pix_fmt != AV_PIX_FMT_YUV420P || desc->comp[0].depth != 8) {
        return AVERROR_INVALIDDATA;
    }
    ctx->frame = av_frame_alloc();
    if (!ctx->frame) return AVERROR(ENOMEM);
    return 0;
}

static av_cold int libbook_encode_close(AVCodecContext *avctx)
{
    libbookContext *ctx = avctx->priv_data;
    av_frame_free(&ctx->frame);
    return 0;
}

static int libbook_receive_packet(AVCodecContext *avctx, AVPacket *pkt)
{
    libbookContext *ctx = avctx->priv_data;
    AVFrame *frame = ctx->frame;
    int ret = ff_encode_get_frame(avctx, frame);
    if (ret < 0 || ret == AVERROR_EOF) return ret;
    int size = frame->width * frame->height;
    ret = ff_get_encode_buffer(avctx, pkt, size, 0);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Could not allocate packet.\n");
        return ret;
    }
    if (frame->linesize[0] == frame->width) {
        memcpy(pkt->data, frame->data[0], size);
    } else {
        int i;
        for (i = 0; i < frame->height; i++) {
            memcpy(pkt->data + frame->width * i,
                frame->data[0] + frame->linesize[0] * i, frame->width);
        }
    }
    return 0;
}

static const FFCodecDefault libbook_defaults[] = {
    { NULL }
};

const enum AVPixelFormat libbook_pix_fmts[] = {
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_NONE
};

static const AVOption options[] = {
    { NULL }
};

static const AVClass class = {
    .class_name = "libbook",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

const FFCodec ff_libbooke_encoder = {
    .p.name           = "libbook",
    .p.long_name      = NULL_IF_CONFIG_SMALL("libbook Encoder"),
    .p.type           = AVMEDIA_TYPE_VIDEO,
    .p.id             = AV_CODEC_ID_BOOK,
    .init           = libbook_encode_init,
    FF_CODEC_RECEIVE_PACKET_CB(libbook_receive_packet),
    .close          = libbook_encode_close,
    .priv_data_size = sizeof(libbookContext),
    .p.priv_class     = &class,
    .defaults       = libbook_defaults,
    .p.pix_fmts       = libbook_pix_fmts,
    .p.capabilities   = AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE,
    .caps_internal  = FF_CODEC_CAP_INIT_CLEANUP,
};
