#ifndef MYCRAFT_DEMO_TEXEL_H
#define MYCRAFT_DEMO_TEXEL_H

#include "utils.h"
#include "glutils.h"
#include "../lib/png_loader.h"

typedef struct texel_pack {
        const char      *filepath;
        image_png       png;

        GLuint          texel;
        int32_t         width;
        int32_t         height;

        int32_t         slice;
        int32_t         slots;

        texel_filter    filter;
} texel_pack;

typedef enum {
        TEXELPACK_GENERIC_BLOCKS = 0,
        NR_TEXEL_PACKS,
} texel_pack_idx;

texel_pack *texel_pack_get(texel_pack_idx idx);

int texel_pack_init(void);
int texel_pack_deinit(void);

int texel_slot_get(texel_pack_idx idx, const ivec2 slot, vec2 *uv);
GLuint texel_pack_texture_get(texel_pack_idx idx);

#endif //MYCRAFT_DEMO_TEXEL_H
