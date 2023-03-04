/**
 * @file
 * BOOK protocol
 */

#define _DEFAULT_SOURCE
#define _BSD_SOURCE     /* Needed for using struct ip_mreq with recent glibc */
#include <pthread.h>

#include "avformat.h"
#include "libavutil/opt.h"
#include "network.h"

typedef struct BookContext {
    const AVClass *class;
    int fd;
    int blocksize;
} BookContext;

#define OFFSET(x) offsetof(BookContext, x)
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
    { "blocksize", "blocksize data size (in bytes)", OFFSET(blocksize),
        AV_OPT_TYPE_INT, { .i64 = 1024 }, 48, INT_MAX, .flags = D|E },
    { NULL }
};

static const AVClass book_class = {
    .class_name = "book",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

/**
 * Return the udp file handle for select() usage to wait for several RTP
 * streams at the same time.
 * @param h media file context
 */
static int book_get_file_handle(URLContext *h)
{
    BookContext *s = h->priv_data;
    return s->fd;
}

static int book_open(URLContext *h, const char *uri, int flags)
{
    BookContext *s = h->priv_data;
    char ip[256];
    int port = 0;
    struct sockaddr_in addr;

    av_url_split(NULL, 0, NULL, 0, ip, sizeof(ip), &port, NULL, 0, uri);
    av_log(h, AV_LOG_INFO, "connecting to %s:%d\n", ip, port);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    if ((s->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (connect(s->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        av_log(h, AV_LOG_ERROR, "Error connecting to %s\n", uri);
        return -1;
    }

    return 0;
}

static int book_read(URLContext *h, uint8_t *buf, int size)
{
    BookContext *s = h->priv_data;
    int ret;
    size = FFMIN(size, s->blocksize);
    ret = read(s->fd, buf, size);
    if (ret == 0) return AVERROR_EOF;
    av_log(h, AV_LOG_INFO, "read %d\n", ret);
    return (ret == -1) ? AVERROR(errno) : ret;
}

static int book_write(URLContext *h, const uint8_t *buf, int size)
{
    BookContext *s = h->priv_data;
    av_log(h, AV_LOG_INFO, "write %d\n", size);
    if (s->fd < 0) return -1;
    return write(s->fd, buf, size);
}

static int book_close(URLContext *h)
{
    BookContext *s = h->priv_data;
    av_log(h, AV_LOG_INFO, "close\n");
    if (s->fd > -1) close(s->fd);
    return 0;
}

const URLProtocol ff_book_protocol = {
    .name                = "book",
    .url_open            = book_open,
    .url_read            = book_read,
    .url_write           = book_write,
    .url_close           = book_close,
    .url_get_file_handle = book_get_file_handle,
    .priv_data_size      = sizeof(BookContext),
    .priv_data_class     = &book_class,
    .flags               = URL_PROTOCOL_FLAG_NETWORK,
};
