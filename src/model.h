#ifndef MYCRAFT_DEMO_MODEL_H
#define MYCRAFT_DEMO_MODEL_H

#include <cglm/cglm.h>

#include "utils.h"
#include "glutils.h"
#include "block.h"

#define VERTICES_TRIANGULATE_CUBE (CUBE_QUAD_FACES * VERTICES_TRIANGULATE_QUAD)

typedef struct block_face {
        vec3 vertex[VERTICES_TRIANGULATE_QUAD];
        vec3 vertex_normal[VERTICES_TRIANGULATE_QUAD];
        vec2 uv[VERTICES_TRIANGULATE_QUAD];
} block_face;

typedef struct block_model {
        vec3            origin_gl;

        gl_attr         glattr;
} block_model;

int block_model_init(block_model *mesh, vec3 origin_gl);
int block_model_deinit(block_model *mesh);
int block_model_generate(block_model *mesh, block_attr *blk_attr);
int block_model_draw(block_model *mesh, mat4 mat_transform);

#endif //MYCRAFT_DEMO_MODEL_H
