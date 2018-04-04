#ifndef MYCRAFT_DEMO_MODEL_H
#define MYCRAFT_DEMO_MODEL_H

#include <cglm/cglm.h>

#include "utils.h"
#include "glutils.h"
#include "block.h"

#define VERTICES_TRIANGULATE_CUBE (CUBE_QUAD_FACES * VERTICES_TRIANGULATE_QUAD)

typedef struct block_face {
        vertex_attr     vertex[VERTICES_TRIANGULATE_QUAD];
} block_face;

typedef struct block_model {
        vec3            origin_gl;

        gl_attr         *glattr;
        gl_vbo          *glvbo;

        block_face      *faces;
} block_model;

int block_model_init(block_model *model, vec3 origin_gl);
int block_model_deinit(block_model *model);

int block_model_faces_alloc(block_model *model);
int block_model_faces_free(block_model *model);
int block_model_faces_generate(block_model *model, block_attr *blk_attr);

vertex_attr *block_model_vertices_pack(block_model *model);
int block_model_face_vertices_copy(block_model *model, vec3 *vertices,
                                   vec3 *normals, vec2 *uvs);

int block_model_gl_attr(block_model *model, block_attr *blk_attr);
int block_model_gl_attr_vbo(block_model *model, block_attr *blk_attr);

int block_model_draw(block_model *model, mat4 mat_transform);
int block_model_draw_indexed(block_model *model, mat4 mat_transform);

#endif //MYCRAFT_DEMO_MODEL_H
