#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <memory.h>

#include <lodepng.h>
#include "../src/debug.h"
#include "png_loader.h"

int image_png24_load(image_png *png, const char *filepath)
{
        uint32_t ret;
        uint32_t width, height;
        uint8_t *data;

        if (png->data) {
                pr_err_func("png data is not clean\n");
        }

        ret = lodepng_decode24_file(&data, &width, &height, filepath);
        if (ret) {
                pr_err_func("failed to load png image\n");
                return -EIO;
        }

        png->width = width;
        png->height = height;
        png->bpp = PNG_BPP_RGB;
        png->data = data;

        return 0;
}

int image_png32_load(image_png *png, const char *filepath)
{
        uint32_t ret;
        uint32_t width, height;
        uint8_t *data;

        if (png->data) {
                pr_err_func("png data is not clean\n");
        }

        ret = lodepng_decode32_file(&data, &width, &height, filepath);
        if (ret) {
                pr_err_func("failed to load png image\n");
                return -EIO;
        }

        png->width = width;
        png->height = height;
        png->bpp = PNG_BPP_RGBA;
        png->data = data;

        return 0;
}

int image_png_free(image_png *png)
{
        if (png->data) {
                free(png->data);
        }

        memset(png, 0x00, sizeof(image_png));

        return 0;
}
