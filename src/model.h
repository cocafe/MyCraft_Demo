#ifndef MYCRAFT_DEMO_MODEL_H
#define MYCRAFT_DEMO_MODEL_H

#include <cglm/cglm.h>

#include "utils.h"
#include "glutils.h"
#include "block.h"

typedef struct block_face {
        vec3            vertex[VERTICES_QUAD_FACE];
        vec3            vertex_normal[VERTICES_QUAD_FACE];
        vec2            uv[VERTICES_QUAD_FACE];

        vec3            normal;

        gl_attr         glattr;
} block_face;

typedef struct block_model {
        vec3            origin_gl;

        block_face      faces[CUBE_QUAD_FACES];
} block_model;

int block_model_init(block_model *mesh, vec3 origin_gl);
int block_model_deinit(block_model *mesh);
int block_model_generate(block_model *mesh, block_attr *blk_attr);
int block_model_draw(block_model *mesh, mat4 mat_transform);

#endif //MYCRAFT_DEMO_MODEL_H
