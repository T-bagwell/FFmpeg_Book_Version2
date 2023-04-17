#include <stdio.h>
#include <libswscale/swscale.h>

AVFrame* alloc_buffered_frame(int w, int h, int fmt)
{
    AVFrame *frame = NULL;
    int ret = 0;
    frame = av_frame_alloc();
    if (!frame) {
        goto failed;
    }

    frame->width = w;
    frame->height = h;
    frame->format = fmt;

    if ((ret = av_frame_get_buffer(frame, 0)) != 0) {
        goto failed;
    } else {
        goto success;
    }

failed:
    if (frame) {
        av_frame_free(&frame);
    }

success:
    return frame;
}

int fill_yuv_from_file(AVFrame *frame, FILE *fp)
{
    int w = frame->width;
    int h = frame->height;

    size_t y_bytes = w * h;
    size_t u_bytes = w * h / 4;
    size_t v_bytes = w * h / 4;
    size_t read_bytes = 0;

    // read Y plane
    read_bytes = fread((void*)frame->data[0], 1, y_bytes, fp);
    if (read_bytes != y_bytes) {
        goto failed;
    }
    // read U plane
    read_bytes = fread((void*)frame->data[1], 1, u_bytes, fp);
    if (read_bytes != u_bytes) {
        goto failed;
    }
    // read V plane
    read_bytes = fread((void*)frame->data[2], 1, v_bytes, fp);
    if (read_bytes != v_bytes) {
        goto failed;
    }

    return 0;

failed:
    return -1;
}

int save_frame_to_file(AVFrame *frame, FILE *fp)
{
    size_t data_size = frame->linesize[0] * frame->height;
    size_t write_bytes = fwrite(frame->data[0], 1, data_size, fp);
    if (write_bytes != data_size) {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    const int src_frame_width  = 1280;
    const int src_frame_height = 720;
    const int dst_frame_width  = 960;
    const int dst_frame_height = 540;
    struct SwsContext * ctx = NULL;

    ctx = sws_getContext(src_frame_width, src_frame_height, AV_PIX_FMT_YUV420P
                   , dst_frame_width, dst_frame_height, AV_PIX_FMT_RGB24, 0, NULL, NULL, NULL);
    if (!ctx) {
        printf("sws_getContext error.\n");
        return -1;
    }

    AVFrame *src_frame = alloc_buffered_frame(src_frame_width, src_frame_height, AV_PIX_FMT_YUV420P);
    AVFrame *dst_frame = alloc_buffered_frame(dst_frame_width, dst_frame_height, AV_PIX_FMT_RGB24);
    if (!src_frame || !dst_frame) {
        printf("alloc frame data failed.\n");
        return -2;
    }

    FILE *yuv_in = fopen("./one_frame.yuv", "r");
    if (!yuv_in) {
        printf("open file failed.\n");
        return -3;
    }

    if (fill_yuv_from_file(src_frame, yuv_in) != 0) {
        printf("read file data error.\n");
        return -4;
    }

    int h = sws_scale(ctx, (const uint8_t**)src_frame->data, src_frame->linesize, 0
              , src_frame_height, dst_frame->data, dst_frame->linesize);

    if (h != dst_frame_height) {
        printf("sws_scale internal error.\n");
        return -5;
    }

    FILE *rgb_out = fopen("./one_frame.rgb", "w");
    if (!rgb_out) {
        printf("open file failed.\n");
        return -6;
    }

    if (save_frame_to_file(dst_frame, rgb_out) != 0) {
        printf("write file failed.\n");
        return -7;
    }

	return 0;
}
