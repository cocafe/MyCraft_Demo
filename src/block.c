#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <GL/glew.h>

#include "utils.h"
#include "debug.h"
#include "glutils.h"
#include "block.h"

const char *texel_filter_level[] = {
        "nearest",
        "linear",
        "linear_mipmap",
        "anisotropic",
};

const char *cube_face_str[] = {
        "front",
        "back",
        "top",
        "bottom",
        "left",
        "right",
};

const char *texel_rotate_str[] = {
        "clockwise_0",
        "clockwise_90",
        "clockwise_180",
        "clockwise_270",
};

static block_shader block_shader_generic = {
        .program = GL_PROGRAM_NONE,
        .shader_vert = BLOCK_SHADER_FILE("block_generic_vertex"),
        .shader_frag = BLOCK_SHADER_FILE("block_generic_fragment"),
};

static block_shader *block_shader_list[] = {
        [BLK_SHADER_GENERIC] = &block_shader_generic,
        NULL,
};

#define BLOCK_DIMENSION_CUBE                                    \
        .size = {                                               \
                .height = BLOCK_EDGE_LEN_GLUNIT,                \
                .length = BLOCK_EDGE_LEN_GLUNIT,                \
                .width = BLOCK_EDGE_LEN_GLUNIT,                 \
        }

#define BLOCK_MODEL_CUBE                                        \
        .size_model = {                                         \
                .height = BLOCK_EDGE_LEN_GLUNIT,                \
                .length = BLOCK_EDGE_LEN_GLUNIT,                \
                .width = BLOCK_EDGE_LEN_GLUNIT,                 \
        }


#define BLOCK_TEXEL_SLOT_DIRT                   {  0, 0  }
#define BLOCK_TEXEL_SLOT_GRASS_SIDE             {  1, 0  }
#define BLOCK_TEXEL_SLOT_GRASS_TOP              {  2, 0  }
#define BLOCK_TEXEL_SLOT_BEDROCK                {  9, 0  }
#define BLOCK_TEXEL_SLOT_STONE                  {  0, 1  }
#define BLOCK_TEXEL_SLOT_TNT_TOP                {  0, 15 }
#define BLOCK_TEXEL_SLOT_TNT_BOTTOM             {  1, 15 }
#define BLOCK_TEXEL_SLOT_TNT_SIDE               {  2, 15 }
#define BLOCK_TEXEL_SLOT_GLASS                  { 11, 15 }
#define BLOCK_TEXEL_SLOT_DEBUG                  { 15, 15 }
#define BLOCK_TEXEL_SLOT_DEBUG2                 { 15, 14 }

#define BLOCK_TEXTURE_NONE                                      \
        .texel = {                                              \
                .textured = 0,                                  \
        }

#define BLOCK_TEXTURE_SINGLE(pack, slot)                       \
        .texel = {                                              \
                .textured = 1,                                  \
                .texel = GL_TEXTURE_NONE,                       \
                .texel_pack = (pack),                           \
                .texel_slot = {                                 \
                        [CUBE_FRONT]  = BLOCK_TEXEL_##slot,     \
                        [CUBE_BACK]   = BLOCK_TEXEL_##slot,     \
                        [CUBE_TOP]    = BLOCK_TEXEL_##slot,     \
                        [CUBE_BOTTOM] = BLOCK_TEXEL_##slot,     \
                        [CUBE_LEFT]   = BLOCK_TEXEL_##slot,     \
                        [CUBE_RIGHT]  = BLOCK_TEXEL_##slot,     \
                },                                              \
        }

#define BLOCK_TEXTURE_TOP_BOTTOM_SIDE(pack, top, bottom, side)  \
        .texel = {                                              \
                .textured = 1,                                  \
                .texel = GL_TEXTURE_NONE,                       \
                .texel_pack = (pack),                           \
                .texel_slot = {                                 \
                        [CUBE_FRONT]  = BLOCK_TEXEL_##side,     \
                        [CUBE_BACK]   = BLOCK_TEXEL_##side,     \
                        [CUBE_TOP]    = BLOCK_TEXEL_##top,      \
                        [CUBE_BOTTOM] = BLOCK_TEXEL_##bottom,   \
                        [CUBE_LEFT]   = BLOCK_TEXEL_##side,     \
                        [CUBE_RIGHT]  = BLOCK_TEXEL_##side,     \
                },                                              \
        }

static block_attr block_test = {
        .name = "testy",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .texel = {
                .textured = 1,
                .texel = GL_TEXTURE_NONE,
                .texel_pack = TEXELPACK_GENERIC_BLOCKS,
                .texel_slot = {
                        [CUBE_FRONT] = BLOCK_TEXEL_SLOT_DEBUG,
                        [CUBE_BACK] = BLOCK_TEXEL_SLOT_DEBUG,
                        [CUBE_TOP] = BLOCK_TEXEL_SLOT_DEBUG,
                        [CUBE_BOTTOM] = BLOCK_TEXEL_SLOT_DEBUG,
                        [CUBE_LEFT] = BLOCK_TEXEL_SLOT_DEBUG,
                        [CUBE_RIGHT] = BLOCK_TEXEL_SLOT_DEBUG,
                },
        },

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block_attr block_air = {
        .name = "air",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        BLOCK_TEXTURE_NONE,

        .visible = 0,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 0,
        .throughable = 1,
};

static block_attr block_air_wall = {
        .name = "air_wall",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        BLOCK_TEXTURE_NONE,

        .visible = 0,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 0,
        .throughable = 0,
};

static block_attr block_bedrock = {
        .name = "bedrock",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        BLOCK_TEXTURE_SINGLE(TEXELPACK_GENERIC_BLOCKS, SLOT_BEDROCK),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 0,
        .throughable = 0,
};

static block_attr block_grass = {
        .name = "grass",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        BLOCK_TEXTURE_TOP_BOTTOM_SIDE(TEXELPACK_GENERIC_BLOCKS, SLOT_GRASS_TOP,
                                      SLOT_DIRT, SLOT_GRASS_SIDE),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block_attr block_dirt = {
        .name = "dirt",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        BLOCK_TEXTURE_SINGLE(TEXELPACK_GENERIC_BLOCKS, SLOT_DIRT),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block_attr block_stone = {
        .name = "stone",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        BLOCK_TEXTURE_SINGLE(TEXELPACK_GENERIC_BLOCKS, SLOT_STONE),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block_attr block_tnt = {
        .name = "TNT",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        BLOCK_TEXTURE_TOP_BOTTOM_SIDE(TEXELPACK_GENERIC_BLOCKS, SLOT_TNT_TOP,
                                      SLOT_TNT_BOTTOM, SLOT_TNT_SIDE),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block_attr block_glass = {
        .name = "glass",

        BLOCK_DIMENSION_CUBE,
        .size_model = {
                .width = 1.0f,
                .length = 0.1f,
                .height = 1.0f,
        },

        BLOCK_TEXTURE_SINGLE(TEXELPACK_GENERIC_BLOCKS, SLOT_GLASS),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block_attr *block_attr_list[] = {
        [BLOCK_AIR]             = &block_air,
        [BLOCK_AIRWALL]         = &block_air_wall,
        [BLOCK_BEDROCK]         = &block_bedrock,
        [BLOCK_GRASS]           = &block_grass,
        [BLOCK_DIRT]            = &block_dirt,
        [BLOCK_STONE]           = &block_stone,
        [BLOCK_TNT]             = &block_tnt,
        [BLOCK_GLASS]           = &block_glass,
        [BLOCK_TEST]            = &block_test,
        NULL,
};

int block_texel_init(block_texel *t)
{
        if (!t->textured)
                return 0;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                vec2 *uv = t->uv[i];

                t->texel_rotation[i] = TEXEL_ROTATE_0;
                t->texel = texel_pack_texture_get(t->texel_pack);

                texel_slot_get(t->texel_pack, t->texel_slot[i], uv);
        }

        return 0;
}

int block_attr_init(void)
{
        int i;
        block_attr *p;
        block_attr **list = block_attr_list;

        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                block_texel_init(&p->texel);
        }

        return 0;
}

int block_attr_deinit(void)
{
        return 0;
}

int block_attr_dump(void)
{
        int i;
        block_attr *p;
        block_attr **list = block_attr_list;

        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                pr_info("Block: %s\n", p->name);
                pr_info("Dimensions: w: %0.2f l: %0.2f h: %0.2f\n",
                        p->size.width, p->size.length, p->size.height);
                pr_info("Visible: %d\n", p->visible);
                pr_info("Destroyable: %d\n", p->destroyable);
                pr_info("Throughable: %d\n", p->throughable);
                pr_info("Textured: %d\n", p->texel.textured);
                pr_info("------\n");
        }

        return 0;
}

block_attr *block_attr_get(block_attr_idx idx)
{
        if (idx < 0 || idx >= NR_BLOCK_TYPE)
                return NULL;

        return block_attr_list[idx];
}

int block_shader_init(void)
{
        int i;
        block_shader *p;
        block_shader **list = block_shader_list;

        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                p->program = program_create(p->shader_vert, p->shader_frag);
                if (glIsProgram(p->program) == GL_FALSE) {
                        pr_err_func("failed to create shader program for %s %s\n",
                                    p->shader_vert, p->shader_frag);
                        return -EFAULT;
                }
        }

        return 0;
}

int block_shader_deinit(void)
{
        int i;
        block_shader *p;
        block_shader **list = block_shader_list;

        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                if (glIsProgram(p->program) != GL_FALSE) {
                        program_delete(&p->program);
                }
        }

        return 0;
}

GLuint block_shader_get(block_shader_idx idx)
{
        if (idx >= NR_BLOCK_SHADER || idx < 0)
                return GL_PROGRAM_NONE;

        return block_shader_list[idx]->program;
}
