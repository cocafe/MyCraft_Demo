#ifndef MYCRAFT_DEMO_MODEL_H
#define MYCRAFT_DEMO_MODEL_H

#include <cglm/cglm.h>

#include "utils.h"
#include "glutils.h"
#include "block.h"

enum vec_attr {
        X = 0,
        Y,
        Z,
        W,
        NR_VEC_IDX,
};

enum vertices {
        V1 = 0,
        V2,
        V3,
        V4,
        V5,
        V6,
};

typedef struct face_block {
        vec3            vertex[VERTICES_QUAD_FACE];
        vec3            vertex_normal[VERTICES_QUAD_FACE];
        vec2            uv[VERTICES_QUAD_FACE];

        vec3            normal;

        gl_attr         glattr;
} face_block;

typedef struct model_block {
        vec3            origin_gl;

        face_block       faces[CUBE_FACES_QUADS];
} model_block;

int model_block_init(model_block *cube, vec3 origin_gl);
int model_block_deinit(model_block *cube);
int model_block_generate(model_block *cube, block_attr *blk_attr);
int model_block_draw(model_block *cube, mat4 mat_transform);

#endif //MYCRAFT_DEMO_MODEL_H
