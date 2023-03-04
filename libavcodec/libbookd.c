#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "libavutil/cpu.h"
#include "libavutil/imgutils.h"
#include "libavutil/timestamp.h"

#include "avcodec.h"
#include "codec_internal.h"
#include "decode.h"
#include "internal.h"
#include "profiles.h"

typedef struct BookDecodeContext {
    // AVClass *class;
} BookDecodeContext;

static av_cold int book_init(AVCodecContext *avctx)
{
    // BookDecodeContext *ctx           = avctx->priv_data;
    av_log(avctx, AV_LOG_INFO, "picture size = %dx%d\n", avctx->width, avctx->height);
    avctx->pix_fmt = AV_PIX_FMT_YUV420P;
    return 0;
}

static int book_decode(AVCodecContext *avctx, AVFrame *data, int *got_frame,
                      AVPacket *pkt)
{
    // BookDecodeContext *ctx = avctx->priv_data;
    AVFrame *picture = data;
    int ret;
    int size = avctx->width * avctx->height;
    if (pkt->size != size) return AVERROR_INVALIDDATA;
    if ((ret = ff_get_buffer(avctx, picture, 0)) < 0) return ret;
    memcpy(picture->data[0], pkt->data, size);
    memset(picture->data[1], 128, size / 4);
    memset(picture->data[2], 128, size / 4);
    picture->key_frame = 1;
    picture->pict_type = AV_PICTURE_TYPE_I;
    av_log(avctx, AV_LOG_INFO, "decode pts [%s]\n", av_ts2str(pkt->pts));
    picture->pts = pkt->pts;
    *got_frame = 1;
    return pkt->size;
}

static av_cold int book_close(AVCodecContext *avctx)
{
    // BookDecodeContext *ctx = avctx->priv_data;
    return 0;
}

const FFCodec ff_libbookd_decoder = {
    .p.name           = "libbookd",
    CODEC_LONG_NAME("BOOK Codec"),
    .p.type           = AVMEDIA_TYPE_VIDEO,
    .p.id             = AV_CODEC_ID_BOOK,
    .priv_data_size   = sizeof(BookDecodeContext),
    .init             = book_init,
    .close            = book_close,
    FF_CODEC_DECODE_CB(book_decode),
    .p.capabilities   = AV_CODEC_CAP_DR1,
};
