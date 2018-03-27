#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <errno.h>

#include "debug.h"
#include "utils.h"

int memzero(void *ptr, size_t size)
{
        if (!ptr)
                return -EINVAL;

        memset(ptr, 0x00, size);

        return 0;
}

char *file_read(const char *filepath)
{
        errno_t err;
        FILE *fp;
        char *buf;
        char ch;
        size_t fsize;
        size_t i;

        err = fopen_s(&fp, filepath, "r");
        if (!fp) {
                pr_err_fopen(filepath, err);
                return NULL;
        }

        fsize = 0;
        while (1) {
                ch = (char)fgetc(fp);

                if (ferror(fp)) {
                        pr_err_func("failed to read file %s\n", filepath);
                        return NULL;
                }

                if (feof(fp)) {
                        break;
                }

                fsize++;
        }

        buf = calloc(1, sizeof(char) * (fsize + 2));
        if (!buf) {
                pr_err_alloc();
                return NULL;
        }

        memset(buf, '\0', sizeof(char) * (fsize + 2));

        rewind(fp);
        i = 0;
        while (1) {
                ch = (char)fgetc(fp);

                if (ferror(fp)) {
                        pr_err_func("failed to read file %s\n", filepath);
                        return NULL;
                }

                if (feof(fp)) {
                        break;
                }

                buf[i] = ch;
                i++;
        }

        return buf;
}

char *buf_alloc(size_t len)
{
        if (!len)
                return NULL;

        char *buf = calloc(1, sizeof(char) * len);
        if (!buf)
                return NULL;

        memset(buf, '\0', sizeof(char) * len);

        return buf;
}

int buf_free(char **buf)
{
        if (!*buf)
                return -EINVAL;

        free(*buf);
        *buf = NULL;

        return 0;
}

void image_vertical_flip(uint8_t *data, uint32_t width, uint32_t height)
{
        size_t size = width * height * 4;
        uint32_t stride = sizeof(int8_t) * width * 4;
        uint8_t *new_data = calloc(1, sizeof(uint8_t) * size);

        for (uint32_t i = 0; i < height; i++) {
                int j = height - i - 1;
                memcpy(new_data + j * stride, data + i * stride, stride);
        }

        memcpy(data, new_data, size);
        free(new_data);
}
