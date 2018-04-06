#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <memory.h>

#include <GL/glew.h>

#include "debug.h"
#include "utils.h"
#include "glutils.h"
#include "block.h"
#include "texel.h"

#define TEXEL_FILTER_DEFAULT            (FILTER_NEAREST)

static texel_pack texel_generic_blocks = {
        .filepath = BLOCK_TEXTURE_FILE("generic_blocks"),
        .filter   = TEXEL_FILTER_DEFAULT,
        .slice    = 16,
};

static texel_pack *texel_pack_list[] = {
        [TEXELPACK_GENERIC_BLOCKS] = &texel_generic_blocks,
        NULL,
};

texel_pack *texel_pack_get(texel_pack_idx idx)
{
        return texel_pack_list[idx];
}

int texel_pack_init(void)
{
        int i;
        texel_pack *p;
        texel_pack **list = texel_pack_list;

        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                image_png32_load(&p->png, p->filepath);
                if (p->png.data == NULL) {
                        pr_err_func("failed to load texture [%s]\n", p->filepath);
                        return -EIO;
                }

                image_vertical_flip(p->png.data,
                                    p->png.width,
                                    p->png.height);

                // Keep a copy of texture width, height
                p->width = p->png.width;
                p->height = p->png.height;

                p->slots = p->slice * p->slice;

                p->texel = texture_png_create(&p->png, p->filter, 0);
                if (glIsTexture(p->texel) == GL_FALSE) {
                        pr_err_func("failed to create texture [%s]\n", p->filepath);
                        return -EFAULT;
                }

                pr_debug_func("loaded texture pack [%d] [%s][%d x %d]\n",
                              p->texel, p->filepath, p->width, p->height);
        }

        return 0;
}

int texel_pack_deinit(void)
{
        int i;
        texel_pack *p;
        texel_pack **list = texel_pack_list;

        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                if (p->png.data != NULL)
                        image_png_free(&p->png);

                texture_delete(&p->texel);
        }

        return 0;
}

// TODO: Handle slot that larger then one

int texel_slot_get(texel_pack_idx idx, const ivec2 slot, vec2 *uv)
{
        texel_pack *pack = texel_pack_get(idx);
        vec2 slot_uv[VERTICES_QUAD];
        float slot_len;

        // Hack to reduce the nearest filtered edge
        float refine_adjust = 0.0003;

        enum {
                LL = V1,
                UL = V2,
                UR = V3,
                LR = V4,
        };

        enum {
                U = 0,
                V,
        };

        if (!pack)
                return -ENODATA;


        if (slot[X] < 0 || slot[X] >= pack->slice)
                return -EINVAL;

        if (slot[Y] < 0 || slot[Y] >= pack->slice)
                return -EINVAL;

        memzero(slot_uv, sizeof(slot_uv));

        slot_len = 1.0f / (float)pack->slice;

        slot_uv[LL][U] = (float)slot[X] / pack->slice;
        slot_uv[LL][V] = (float)slot[Y] / pack->slice;

        slot_uv[UL][U] = slot_uv[LL][U];
        slot_uv[UL][V] = slot_uv[LL][V] + slot_len;

        slot_uv[UR][U] = slot_uv[LL][U] + slot_len;
        slot_uv[UR][V] = slot_uv[LL][V] + slot_len;

        slot_uv[LR][U] = slot_uv[LL][U] + slot_len;
        slot_uv[LR][V] = slot_uv[LL][V];

        slot_uv[LL][U] += refine_adjust;
        slot_uv[LL][V] += refine_adjust;

        slot_uv[UL][U] += refine_adjust;
        slot_uv[UL][V] -= refine_adjust;

        slot_uv[UR][U] -= refine_adjust;
        slot_uv[UR][V] -= refine_adjust;

        slot_uv[LR][U] -= refine_adjust;
        slot_uv[LR][V] += refine_adjust;

        // FIXME: may overflow in parameter uv
        memcpy(uv, slot_uv, sizeof(slot_uv));

        return 0;
}

GLuint texel_pack_texture_get(texel_pack_idx idx)
{
        texel_pack *pack = texel_pack_get(idx);

        if (!pack)
                return GL_TEXTURE_NONE;

        return pack->texel;
}
