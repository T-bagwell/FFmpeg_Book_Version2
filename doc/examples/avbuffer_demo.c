/*
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * libavformat API example.
 *
 * Output a media file in any supported libavformat format. The default
 * codecs are used.
 * @example avframe_demo.c
 */

#include <libavutil/error.h>
#include <libavutil/buffer.h>
#include <libavutil/mem.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    uint8_t *data = av_malloc(1024);

    AVBufferRef *fdd_buf = av_buffer_create(data, 1024, NULL, NULL, AV_BUFFER_FLAG_READONLY);
    if (!fdd_buf)
        return AVERROR(ENOMEM);

    fprintf(stderr, "fdd_buf writable = %d\n", av_buffer_is_writable(fdd_buf));

    av_buffer_ref(fdd_buf);
    av_buffer_ref(fdd_buf);

    fprintf(stderr, "fdd count = %d\n", av_buffer_get_ref_count(fdd_buf));
    av_buffer_make_writable(&fdd_buf);
    fprintf(stderr, "fdd_buf writable = %d\n", av_buffer_is_writable(fdd_buf));

    return 0;
}
