#ifndef MYCRAFT_DEMO_MODEL_H
#define MYCRAFT_DEMO_MODEL_H

#include <cglm/cglm.h>

#include "utils.h"
#include "glutils.h"
#include "block.h"

#define CUBE_FACE_UL                            (V1)
#define CUBE_FACE_UR                            (V2)
#define CUBE_FACE_LL                            (V3)
#define CUBE_FACE_LR                            (V5)

typedef struct block_face {
        vertex_attr     *vertices;
        vec3            normal;
        int             visible;
} block_face;

typedef struct block_model {
        vec3            origin_gl;

        block_face      faces[CUBE_QUAD_FACES];
} block_model;

int block_model_face_generate(block_face *f, block_model *m, block_attr *attr,
                              float scale, int idx);
int block_model_face_init(block_face *f);
int block_model_face_deinit(block_face *f);

int block_model_generate(block_model *model, block_attr *blk_attr, float scale);
int block_model_init(block_model *model, vec3 origin_gl);
int block_model_deinit(block_model *model);

int block_wireframe_draw(ivec3 origin_l, vec4 color, int invert_color, mat4 mat_transform);

#endif //MYCRAFT_DEMO_MODEL_H
