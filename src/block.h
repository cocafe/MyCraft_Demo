#ifndef MYCRAFT_DEMO_BLOCK_H
#define MYCRAFT_DEMO_BLOCK_H

#include "glutils.h"
#include "texel.h"
#include "../lib/png_loader.h"

#define BLOCK_EDGE_LEN_GLUNIT           (1U)

#define CUBE_QUAD_FACES                 (6)

#define BLOCK_SHADER_FOLDER             "resource/shader/"
#define BLOCK_SHADER_FILE(file)         (BLOCK_SHADER_FOLDER file SUFFIX_GLSL)

#define BLOCK_TEXTURE_FOLDER            "resource/texture/block/"
#define BLOCK_TEXTURE_FILE(file)        (BLOCK_TEXTURE_FOLDER file SUFFIX_PNG)

/**
 *        | +Y (height)
 *        |
 *        |
 *        |_______ +X (width)
 *        /
 *       /
 *      / +Z (length)
 */
typedef struct dimension {
        float width;    // X
        float length;   // Z
        float height;   // Y
} dimension;

typedef enum cube_face_idx {
        CUBE_FRONT = 0,
        CUBE_BACK,
        CUBE_TOP,
        CUBE_BOTTOM,
        CUBE_LEFT,
        CUBE_RIGHT,
        NR_CUBE_FACES,
} cube_face_idx;

// Texture mapping rotation: clock wise
typedef enum texel_rotate {
        TEXEL_ROTATE_0 = 0,
        TEXEL_ROTATE_90,
        TEXEL_ROTATE_180,
        TEXEL_ROTATE_270,
        NR_TEXEL_ROTATE,
} texel_rotate;

typedef enum block_shader_idx {
        BLK_SHADER_GENERIC = 0,
        NR_BLOCK_SHADER,
} block_shader_idx;

typedef enum block_attr_idx {
        BLOCK_AIR = 0,
        BLOCK_AIRWALL,
        BLOCK_BEDROCK,
        BLOCK_GRASS,
        BLOCK_DIRT,
        BLOCK_STONE,
        BLOCK_TNT,
        BLOCK_GLASS,
        BLOCK_TEST,
        NR_BLOCK_TYPE,
} block_attr_idx;

typedef struct block_shader {
        GLuint          program;
        const char      *shader_vert;
        const char      *shader_frag;
} block_shader;

typedef struct block_texel {
        int32_t         textured;

        GLuint          texel;
        texel_pack_idx  texel_pack;
        ivec2           texel_slot[CUBE_QUAD_FACES];
        texel_rotate    texel_rotation[CUBE_QUAD_FACES];

        // LL[0] -> UL -> UR -> LR
        vec2            uv[CUBE_QUAD_FACES][VERTICES_QUAD];
} block_texel;

typedef struct block_attr {
        const char              *name;

        dimension               size;         // Volume considers in world
        dimension               size_model;   // Actual volume displays

        block_texel             texel;

        int32_t                 visible;        // If invisible, shader is useless
        block_shader_idx        shader;

        int32_t                 throughable;
        int32_t                 destroyable;
} block_attr;

extern const char *texel_filter_level[];
extern const char *cube_face_str[];
extern const char *texel_rotate_str[];

int block_attr_init(void);
int block_attr_deinit(void);
int block_attr_dump(void);
block_attr *block_attr_get(block_attr_idx idx);

int block_shader_init(void);
int block_shader_deinit(void);
GLuint block_shader_get(block_shader_idx idx);

#endif //MYCRAFT_DEMO_BLOCK_H
