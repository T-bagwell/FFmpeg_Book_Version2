/*
 * Copyright (c) 2023 Steven Liu <lq@chinaffmpeg.org>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "config_components.h"

#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/eval.h"
#include "avfilter.h"
#include "drawutils.h"
#include "formats.h"
#include "internal.h"
#include "video.h"

static const char *const var_names[] = {
    "h",            // 图像的边的高
    "n",            // 帧序列数
    "pos",          // 在文件的位置
    "t",            // 单位为秒的时间
    NULL
};

enum var_name {
    VAR_H,
    VAR_N,
    VAR_POS,
    VAR_T,
    VAR_VARS_NB
};

typedef struct BookContext {
    const AVClass *class;
    int h;

    double var_values[VAR_VARS_NB];
    char *h_expr;
    AVExpr *h_pexpr;

    int (*do_slice)(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs);
} BookContext;

static const enum AVPixelFormat pixel_fmts[] = {
    AV_PIX_FMT_ARGB,
    AV_PIX_FMT_NONE
};

static inline int normalize_xy(double d, int chroma_sub)
{
    if (isnan(d))
        return INT_MAX;
    return (int)d & ~((1 << chroma_sub) - 1);
}

static void eval_expr(AVFilterContext *ctx)
{
    BookContext *s = ctx->priv;

    s->var_values[VAR_H] = av_expr_eval(s->h_pexpr, s->var_values, NULL);
    s->h = normalize_xy(s->var_values[VAR_H], 1);
}

static int set_expr(AVExpr **pexpr, const char *expr, const char *option, void *log_ctx)
{
    int ret;
    AVExpr *old = NULL;

    if (*pexpr)
        old = *pexpr;
    ret = av_expr_parse(pexpr, expr, var_names,
                        NULL, NULL, NULL, NULL, 0, log_ctx);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR,
        "Error when evaluating the expression '%s' for %s\n",
               expr, option);
        *pexpr = old;
        return ret;
    }

    av_expr_free(old);
    return 0;
}

// jobnr 是任务号，nb_jobs是任务总数
static int book_do_slice(AVFilterContext *flt_ctx, void *arg, int jobnr, int nb_jobs)
{
    int j = 0;
    AVFrame *frame = arg;   // AVFrame指针
    BookContext *ctx = flt_ctx->priv; // 获取原来的BookContext
    // 根据任务号计算切片的起始位置
    const int slice_start = ((frame->height - ctx->h)/ 2 * jobnr) / nb_jobs;
    const int slice_end = ((frame->height - ctx->h)/ 2 * (jobnr + 1)) / nb_jobs;
    char *p = frame->data[0] +        // 获取数据指针
            slice_start * frame->linesize[0] +
            frame->linesize[0] * frame->height / 2;

    for (int y = slice_start; y < slice_end; y++) { // 遍历切片中的每一行
        for(j = 0; j < frame->linesize[0]; j += 4) { // 遍历行中的每个像素
            p[j    ] = 0x00; // Alpha
            p[j + 1] = 0x00; // R 红
            p[j + 2] = 0xFF; // G 绿
            p[j + 3] = 0x00; // B 蓝
        }
        p += frame->linesize[0]; // 指向下一行
    }

    return 0;
}

static av_cold int config_input(AVFilterLink *inlink)
{
    int ret = 0;
    AVFilterContext *avctx = inlink->dst;
    BookContext *ctx = avctx->priv;

    ctx->do_slice = book_do_slice;

    ctx->var_values[VAR_H]   = NAN;
    ctx->var_values[VAR_N]   = 0;
    ctx->var_values[VAR_POS] = NAN;
    ctx->var_values[VAR_T]   = NAN;

    if ((ret = set_expr(&ctx->h_pexpr, ctx->h_expr, "h", avctx)) < 0) return ret;

    return 0;
}


static int filter_frame(AVFilterLink *link, AVFrame *frame)
{
    AVFilterContext *avctx = link->dst;
    BookContext *ctx = avctx->priv;
    int res;

    ctx->var_values[VAR_H] = frame->height;
    ctx->var_values[VAR_N] = link->frame_count_out;
    ctx->var_values[VAR_T] = frame->pts == AV_NOPTS_VALUE ? NAN : frame->pts * av_q2d(link->time_base);
    ctx->var_values[VAR_POS] = frame->pkt_pos == -1 ? NAN : frame->pkt_pos;
    eval_expr(avctx);

    if (res = ff_filter_execute(avctx, ctx->do_slice, frame, NULL,
                                FFMIN(frame->height, ff_filter_get_nb_threads(avctx))))
        return res;

    return ff_filter_frame(link->dst->outputs[0], frame);
}

static const AVFilterPad avfilter_vf_book_inputs[] = {
    {
        .name             = "default",
        .type             = AVMEDIA_TYPE_VIDEO,
        .filter_frame     = filter_frame,
        .config_props     = config_input,
    },
};

static const AVFilterPad avfilter_vf_book_outputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO,
    },
};

#define OFFSET(x) offsetof(BookContext, x)
static const AVOption book_options[] = {
    { "h", "set the h expression of the picture", OFFSET(h_expr), AV_OPT_TYPE_STRING, { .str = "0" }, 0, 0, AV_OPT_FLAG_FILTERING_PARAM | AV_OPT_FLAG_VIDEO_PARAM },
    { NULL },
};

AVFILTER_DEFINE_CLASS(book);

const AVFilter ff_vf_book = {
    .name        = "book",
    .description = NULL_IF_CONFIG_SMALL("Book the input video vertically."),
    .priv_size   = sizeof(BookContext),
    .priv_class  = &book_class,
    FILTER_INPUTS(avfilter_vf_book_inputs),
    FILTER_OUTPUTS(avfilter_vf_book_outputs),
    FILTER_PIXFMTS_ARRAY(pixel_fmts),
    .flags       = AVFILTER_FLAG_SUPPORT_TIMELINE|AVFILTER_FLAG_SLICE_THREADS,
};

