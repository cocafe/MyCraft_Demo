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

#define BLOCK_TEXTURE_NONE                                      \
        .texels = {                                             \
                [CUBE_FRONT] = {                                \
                        .filepath = NULL,                       \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_BACK] = {                                 \
                        .filepath = NULL,                       \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_TOP] = {                                  \
                        .filepath = NULL,                       \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_BOTTOM] = {                               \
                        .filepath = NULL,                       \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_LEFT] = {                                 \
                        .filepath = NULL,                       \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_RIGHT] = {                                \
                        .filepath = NULL,                       \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
        }

#define BLOCK_TEXTURE_SINGLE(file)                              \
        .texels = {                                             \
                [CUBE_FRONT] = {                                \
                        .filepath = BLOCK_TEXTURE_FILE(file),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_BACK] = {                                 \
                        .filepath = BLOCK_TEXTURE_FILE(file),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_TOP] = {                                  \
                        .filepath = BLOCK_TEXTURE_FILE(file),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_BOTTOM] = {                               \
                        .filepath = BLOCK_TEXTURE_FILE(file),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_LEFT] = {                                 \
                        .filepath = BLOCK_TEXTURE_FILE(file),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_RIGHT] = {                                \
                        .filepath = BLOCK_TEXTURE_FILE(file),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
        }

#define BLOCK_TEXTURE_TOP_BOTTOM_SIDE(top, bottom, side)        \
        .texels = {                                             \
                [CUBE_FRONT] = {                                \
                        .filepath = BLOCK_TEXTURE_FILE(side),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_BACK] = {                                 \
                        .filepath = BLOCK_TEXTURE_FILE(side),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_TOP] = {                                  \
                        .filepath = BLOCK_TEXTURE_FILE(top),    \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_BOTTOM] = {                               \
                        .filepath = BLOCK_TEXTURE_FILE(bottom), \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_LEFT] = {                                 \
                        .filepath = BLOCK_TEXTURE_FILE(side),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
                [CUBE_RIGHT] = {                                \
                        .filepath = BLOCK_TEXTURE_FILE(side),   \
                        .texture  = GL_TEXTURE_NONE,            \
                        .rotation = TEXEL_ROTATE_0,             \
                },                                              \
        }

#define BLOCK_TEXTURE_SIX(front, back, top, bottom, left, right)        \
        .texels = {                                                     \
                [CUBE_FRONT] = {                                        \
                        .filepath = BLOCK_TEXTURE_FILE(front),          \
                        .texture  = GL_TEXTURE_NONE,                    \
                        .rotation = TEXEL_ROTATE_0,                     \
                },                                                      \
                [CUBE_BACK] = {                                         \
                        .filepath = BLOCK_TEXTURE_FILE(back),           \
                        .texture  = GL_TEXTURE_NONE,                    \
                        .rotation = TEXEL_ROTATE_0,                     \
                },                                                      \
                [CUBE_TOP] = {                                          \
                        .filepath = BLOCK_TEXTURE_FILE(top),            \
                        .texture  = GL_TEXTURE_NONE,                    \
                        .rotation = TEXEL_ROTATE_0,                     \
                },                                                      \
                [CUBE_BOTTOM] = {                                       \
                        .filepath = BLOCK_TEXTURE_FILE(bottom),         \
                        .texture  = GL_TEXTURE_NONE,                    \
                        .rotation = TEXEL_ROTATE_0,                     \
                },                                                      \
                [CUBE_LEFT] = {                                         \
                        .filepath = BLOCK_TEXTURE_FILE(left),           \
                        .texture  = GL_TEXTURE_NONE,                    \
                        .rotation = TEXEL_ROTATE_0,                     \
                },                                                      \
                [CUBE_RIGHT] = {                                        \
                        .filepath = BLOCK_TEXTURE_FILE(right),          \
                        .texture  = GL_TEXTURE_NONE,                    \
                        .rotation = TEXEL_ROTATE_0,                     \
                },                                                      \
        }

#define BLOCK_DIMENSION_CUBE                                    \
        .volume = {                                             \
                .height = BLOCK_EDGE_LEN_GLUNIT,               \
                .length = BLOCK_EDGE_LEN_GLUNIT,               \
                .width = BLOCK_EDGE_LEN_GLUNIT,                \
        }

#define BLOCK_MODEL_CUBE                                        \
        .volume_model = {                                       \
                .height = BLOCK_EDGE_LEN_GLUNIT,               \
                .length = BLOCK_EDGE_LEN_GLUNIT,               \
                .width = BLOCK_EDGE_LEN_GLUNIT,                \
        }

static block block_test = {
        .name = "testy",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 1,
        BLOCK_TEXTURE_SIX("test_front", "test_back",
                          "test_top", "test_bottom",
                          "test_left", "test_right"),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block block_air = {
        .name = "air",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 0,
        BLOCK_TEXTURE_NONE,

        .visible = 0,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 0,
        .throughable = 1,
};

static block block_air_wall = {
        .name = "air_wall",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 0,
        BLOCK_TEXTURE_NONE,

        .visible = 0,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 0,
        .throughable = 0,
};

static block block_bedrock = {
        .name = "bedrock",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 1,
        BLOCK_TEXTURE_SINGLE("bedrock"),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block block_grass = {
        .name = "grass",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 1,
        BLOCK_TEXTURE_TOP_BOTTOM_SIDE("grass_top", "dirt", "grass_side"),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block block_grass_path = {
        .name = "grass",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 1,
        BLOCK_TEXTURE_TOP_BOTTOM_SIDE("grass_path_top",
                                      "dirt",
                                      "grass_path_side"),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block block_dirt = {
        .name = "dirt",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 1,
        BLOCK_TEXTURE_SINGLE("dirt"),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block block_stone = {
        .name = "stone",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 1,
        BLOCK_TEXTURE_SINGLE("stone"),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block block_tnt = {
        .name = "TNT",

        BLOCK_DIMENSION_CUBE,
        BLOCK_MODEL_CUBE,

        .have_texel = 1,
        BLOCK_TEXTURE_TOP_BOTTOM_SIDE("tnt_top", "tnt_bottom", "tnt_side"),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block block_glass = {
        .name = "glass",

        BLOCK_DIMENSION_CUBE,
        .volume_model = {
                .width = 1.0f,
                .length = 0.1f,
                .height = 1.0f,
        },

        .have_texel = 1,
        BLOCK_TEXTURE_SINGLE("glass"),

        .visible = 1,
        .shader = BLK_SHADER_GENERIC,

        .destroyable = 1,
        .throughable = 0,
};

static block *block_type_list[] = {
        [BLOCK_AIR]             = &block_air,
        [BLOCK_AIRWALL]         = &block_air_wall,
        [BLOCK_BEDROCK]         = &block_bedrock,
        [BLOCK_GRASS]           = &block_grass,
        [BLOCK_GRASS_PATH]      = &block_grass_path,
        [BLOCK_DIRT]            = &block_dirt,
        [BLOCK_STONE]           = &block_stone,
        [BLOCK_TNT]             = &block_tnt,
        [BLOCK_GLASS]           = &block_glass,
        [BLOCK_TEST]            = &block_test,
        NULL,
};

int block_type_init(void)
{
        int i;
        block *p;
        block **list = block_type_list;

        // TODO: We may want a texture manager to save some memory
        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                if (p->have_texel) {
                        for (int j = 0; j < NR_CUBE_FACES; ++j) {
                                block_texel *texel = &(p->texels[j]);

                                // FIXME: We assume that all png texture has alpha channel (RGBA)
                                image_png32_load(&(texel->png), texel->filepath);
                                if (texel->png.data == NULL) {
                                        pr_err_func("failed to load block [%s] texture\n", p->name);
                                        return -EIO;
                                }

                                // Decoded PNG image need to flip vertically
                                image_vertical_flip(texel->png.data,
                                                    texel->png.width,
                                                    texel->png.height);

                                // TODO: We may wanna change filter level on the fly
                                texel->filter_level = FILTER_NEAREST;
                                texel->texture = texture_png_create(&(texel->png), texel->filter_level);

                                if (glIsTexture(texel->texture) == GL_FALSE) {
                                        pr_err_func("failed to create texture block [%s] face [%s]\n", p->name, cube_face_str[j]);
                                        return -EFAULT;
                                }

                                pr_debug_func("loaded block [%s] [%s] [%s]\n",
                                              p->name, cube_face_str[j],
                                              texel->filepath);
                        }
                }
        }

        return 0;
}

int block_type_deinit(void)
{
        int i;
        block *p;
        block **list = block_type_list;

        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                if (p->have_texel) {
                        for (int j = 0; j < NR_CUBE_FACES; ++j) {
                                if (p->texels[j].png.data)
                                        image_png_free(&(p->texels[j].png));

                                texture_delete(&(p->texels[j].texture));
                        }
                }
        }

        return 0;
}

int block_type_dump(void)
{
        int i;
        block *p;
        block **list = block_type_list;

        pr_info_func("\n");

        for (i = 0, p = list[i]; p != NULL; i++, p = list[i]) {
                pr_info("------\n");
                pr_info("Block: %s\n", p->name);
                pr_info("Dimensions: w: %0.2f l: %0.2f h: %0.2f\n",
                        p->volume.width, p->volume.length, p->volume.height);
                pr_info("Visible: %d\n", p->visible);
                pr_info("Destroyable: %d\n", p->destroyable);
                pr_info("Throughable: %d\n", p->throughable);
                pr_info("Textured: %d\n", p->have_texel);
                if (p->have_texel) {
                        for (int j = 0; j < NR_CUBE_FACES; ++j) {
                                block_texel *texel = &(p->texels[j]);

                                pr_info("Face %d %s:\n", j, cube_face_str[j]);
                                pr_info("Texture PNG: w: %d h: %d bpp: %d\n",
                                        texel->png.width,
                                        texel->png.height,
                                        texel->png.bpp);
                                pr_info("Texture Handle: %d\n",
                                        texel->texture);
                                pr_info("Texture Rotation: %s\n",
                                        texel_rotate_str[texel->rotation]);
                                pr_info("Texture Filter: %s\n",
                                        texel_filter_level[texel->filter_level]);
                        }
                }

        }

        return 0;
}

int block_type_sanity_check()
{
        // TODO: Block sanity check

        return 0;
}

block *block_type_get(int idx)
{
        if (idx >= NR_BLOCK_TYPE || idx < 0)
                return NULL;

        return block_type_list[idx];
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

GLuint block_shader_get(int32_t idx)
{
        if (idx >= NR_BLOCK_SHADER || idx < 0)
                return GL_PROGRAM_NONE;

        return block_shader_list[idx]->program;
}
