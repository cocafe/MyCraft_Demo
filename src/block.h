#ifndef MYCRAFT_DEMO_BLOCK_H
#define MYCRAFT_DEMO_BLOCK_H

#include "glutils.h"
#include "../lib/png_loader.h"

#define BLOCK_EDGE_LEN_GLUNIT           (1U)

#define VERTICES_TRIANGLE               (3)
#define VERTICES_QUAD_FACE              (2 * VERTICES_TRIANGLE)

#define CUBE_FACES_QUADS                (6)

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

typedef enum block_shader_type {
        BLK_SHADER_GENERIC = 0,
        NR_BLOCK_SHADER,
} block_shader_type;

typedef enum block_type {
        BLOCK_AIR = 0,
        BLOCK_AIRWALL,
        BLOCK_BEDROCK,
        BLOCK_GRASS,
        BLOCK_GRASS_PATH,
        BLOCK_DIRT,
        BLOCK_STONE,
        BLOCK_TNT,
        BLOCK_GLASS,
        BLOCK_TEST,
        NR_BLOCK_TYPE,
} block_type;

typedef struct block_shader {
        GLuint          program;
        const char      *shader_vert;
        const char      *shader_frag;
} block_shader;

typedef struct block_texel {
        const char      *filepath;
        image_png       png;
        GLuint          texture;
        texel_rotate    rotation;
        texel_filter    filter_level;
} block_texel;

typedef struct block {
        const char              *name;

        dimension               size;         // Volume considers in world
        dimension               size_model;   // Actual volume displays

        int32_t                 have_texel;
        block_texel             texels[CUBE_FACES_QUADS];

        int32_t                 visible;        // If invisible, shader is useless
        block_shader_type       shader;

        int32_t                 throughable;
        int32_t                 destroyable;
} block;

extern const char *texel_filter_level[];
extern const char *cube_face_str[];
extern const char *texel_rotate_str[];

int block_type_init(void);
int block_type_deinit(void);
int block_type_dump(void);
block *block_type_get(int idx);

int block_shader_init(void);
int block_shader_deinit(void);
GLuint block_shader_get(int32_t idx);

#endif //MYCRAFT_DEMO_BLOCK_H
