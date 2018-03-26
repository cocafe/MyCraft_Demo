#ifndef MYCRAFT_DEMO_PNG_LOADER_H
#define MYCRAFT_DEMO_PNG_LOADER_H

#include <stdint.h>

#define PNG_BPP_RGB                     (24)
#define PNG_BPP_RGBA                    (32)

typedef struct image_png {
        uint32_t        width;
        uint32_t        height;
        int32_t         bpp;
        uint8_t         *data;
} image_png;

int image_png24_load(image_png *png, const char *filepath);
int image_png32_load(image_png *png, const char *filepath);
int image_png_free(image_png *png);

#endif //MYCRAFT_DEMO_PNG_LOADER_H
