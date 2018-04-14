#ifndef MYCRAFT_DEMO_MODEL_H
#define MYCRAFT_DEMO_MODEL_H

#include <cglm/cglm.h>

#include "utils.h"
#include "glutils.h"
#include "block.h"

typedef struct block_face {
        vertex_attr     *vertices;
        vec3            normal;
        int             visible;
} block_face;

typedef struct block_model {
        vec3            origin_gl;

        block_face      faces[CUBE_QUAD_FACES];
} block_model;

int block_model_face_generate(block_face *f, block_model *m,
                              block_attr *attr, int idx);
int block_model_face_init(block_face *f);
int block_model_face_deinit(block_face *f);

int block_model_generate(block_model *model, block_attr *blk_attr);
int block_model_init(block_model *model, vec3 origin_gl);
int block_model_deinit(block_model *model);

int block_wireframe_draw(ivec3 origin_l, vec4 color, int invert_color, mat4 mat_transform);

#endif //MYCRAFT_DEMO_MODEL_H
