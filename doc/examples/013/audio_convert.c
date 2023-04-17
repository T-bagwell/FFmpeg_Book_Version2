#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>

#define IN_CHANNEL (2)
#define OUT_CHANNEL (1)
#define IN_SAMPLERATE (48000)
#define OUT_SAMPLERATE (16000)

#define MAX_OUT_SAMPLES (1024)

static int read_pcm(FILE *fp, char *buf, int buf_size)
{
    int read_bytes = 0;

    read_bytes = fread((void*)buf, 1, buf_size, fp);
    if (read_bytes == buf_size) {
        return buf_size;
    }

    // eof set
    if (feof(fp) != 0) {
        return read_bytes;
    }

    // error set
    if (ferror(fp) != 0) {
        return -1;
    }

    return 0;
}

static int write_pcm(FILE *fp, char *in_buf, int write_size)
{
    int write_bytes = 0;
    write_bytes = fwrite((void*)in_buf, 1, write_size, fp);
    if (write_bytes == write_size) {
        return write_size;
    }

    return -1;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    SwrContext *swr = NULL;

    AVChannelLayout out_ch;
    av_channel_layout_default(&out_ch, OUT_CHANNEL);

    AVChannelLayout in_ch;
    av_channel_layout_default(&in_ch, IN_CHANNEL);

    // create SwrContext and set parameters
    ret = swr_alloc_set_opts2(&swr, &out_ch, AV_SAMPLE_FMT_S16,
                        OUT_SAMPLERATE, &in_ch, AV_SAMPLE_FMT_S16,
                        IN_SAMPLERATE, 0, NULL);
    if (ret != 0) {
        fprintf(stderr, "swr_alloc_set_opts2() Failed.\n");
        exit(-1);
    }

    // init SwrContext
    ret = swr_init(swr);
    if (ret != 0) {
        fprintf(stderr, "swr_init() Failed.\n");
        exit(-1);
    }

    // open in pcm file
    FILE *in_file = fopen(argv[1], "rb");
    if (!in_file) {
        fprintf(stderr, "open in pcm file error, path=%s\n", argv[1]);
        exit(-1);
    }

    // open out pcm file
    FILE *out_file = fopen(argv[2], "wb");
    if (!out_file) {
        fprintf(stderr, "open out pcm file error, path=%s\n", argv[2]);
        exit(-1);
    }

    int in_byte_per_sample = 2;
    int out_byte_per_sample = 2;

    int in_samples_per_channel = 1024;
    int out_samples_per_channel = MAX_OUT_SAMPLES;

    int in_buf_size = IN_CHANNEL * in_samples_per_channel * in_byte_per_sample;
    int out_buf_size = OUT_CHANNEL * out_samples_per_channel * out_byte_per_sample;

    uint8_t *in_buf = (uint8_t*)malloc(in_buf_size);
    uint8_t *out_buf = (uint8_t*)malloc(out_buf_size);

    for (;;) {
        int read_bytes = read_pcm(in_file, in_buf, in_buf_size);
        if (read_bytes <= 0) {
            break;
        }

        // convert
        uint8_t *out[4] = {0};
        out[0] = out_buf;

        uint8_t *in[4] = {0};
        in[0] = in_buf;

        int ret_samples = swr_convert(swr, out, out_samples_per_channel, in, in_samples_per_channel);
        if (ret_samples < 0) {
            break;
        }

        if (ret_samples > 0) {
            int write_size = ret_samples * out_byte_per_sample * OUT_CHANNEL;
            int write_bytes = write_pcm(out_file, out_buf, write_size);

            if (write_bytes <= 0) {
                break;
            }
        }
    }

    swr_free(&swr);
    free(in_buf);
    free(out_buf);

    return 0;
}
