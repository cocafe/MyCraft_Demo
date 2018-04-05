#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <memory.h>

#include <GL/glew.h>

#include "block.h"
#include "debug.h"
#include "utils.h"
#include "glutils.h"
#include "model.h"

enum texel_corner {
        LOWER_LEFT = 0,
        LOWER_RIGHT,
        UPPER_LEFT,
        UPPER_RIGHT,
        NR_TEXEL_CORNER,
};

typedef enum quad_vertices {
        UL  = V1,
        UR  = V2,
        LL  = V3,
        LR  = V5,

        UR1 = V4,
        LL1 = V6,
} quad_vertices;

static vec3 cube_normals[] = {
        [CUBE_FRONT]    = {  0.0f,  0.0f,  1.0f },
        [CUBE_BACK]     = {  0.0f,  0.0f, -1.0f },
        [CUBE_TOP]      = {  0.0f,  1.0f,  0.0f },
        [CUBE_BOTTOM]   = {  0.0f, -1.0f,  0.0f },
        [CUBE_LEFT]     = { -1.0f,  0.0f,  0.0f },
        [CUBE_RIGHT]    = {  1.0f,  0.0f,  0.0f },
};

int block_model_face_init(block_model *model)
{
        size_t face_vertices = VERTICES_TRIANGULATE_QUAD;

        if (!model)
                return -EINVAL;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_face *f = &(model->faces[i]);

                if (!f->visible)
                        continue;

                f->vertices = memalloc(sizeof(vertex_attr) * face_vertices);
                if (!f->vertices) {
                        pr_err_alloc();
                        return -ENOMEM;
                }
        }

        return 0;
}

int block_model_face_deinit(block_model *model)
{
        if (!model)
                return -EINVAL;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_face *f = &(model->faces[i]);

                if (f->vertices)
                        memfree((void **)&f->vertices);
        }

        return 0;
}

static void block_model_face_vertex(block_face *face, block_attr *blk_attr,
                                    const vec3 origin_gl, int face_idx)
{
        float w = blk_attr->size_model.width;
        float h = blk_attr->size_model.height;
        float l = blk_attr->size_model.length;
        vertex_attr *v = face->vertices;

        switch (face_idx) {
                case CUBE_FRONT:
                        v[UL].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[UL].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UL].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[UR].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[UR].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UR].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[LL].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[LL].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LL].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[LR].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[LR].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LR].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_BACK:
                        v[UL].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[UL].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UL].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[UR].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[UR].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UR].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[LL].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[LL].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LL].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[LR].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[LR].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LR].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        break;

                case CUBE_TOP:
                        v[UL].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[UL].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UL].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[UR].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[UR].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UR].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[LL].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[LL].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[LL].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[LR].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[LR].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[LR].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_BOTTOM:
                        v[UL].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[UL].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[UL].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[UR].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[UR].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[UR].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[LL].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[LL].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LL].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[LR].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[LR].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LR].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        break;

                case CUBE_LEFT:
                        v[UL].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[UL].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UL].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[UR].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[UR].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UR].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[LL].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[LL].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LL].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[LR].position[X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        v[LR].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LR].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_RIGHT:
                        v[UL].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[UL].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UL].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[UR].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[UR].position[Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        v[UR].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        v[LL].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[LL].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LL].position[Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        v[LR].position[X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        v[LR].position[Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        v[LR].position[Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        break;

                default:
                        break;
        }

        // Duplicate vertices
        memcpy(v[UR1].position, v[UR].position, sizeof(vec3));
        memcpy(v[LL1].position, v[LL].position, sizeof(vec3));
}

static void block_model_face_uv(block_face *face, block_attr *blk_attr,
                                int face_idx)
{
        int rotation = blk_attr->texel.texel_rotation[face_idx];
        int rotated_seq[][4] = {
                [TEXEL_ROTATE_0]   = { LL, UL, UR, LR },
                [TEXEL_ROTATE_90]  = { LR, LL, UL, UR },
                [TEXEL_ROTATE_180] = { UR, LR, LL, UL },
                [TEXEL_ROTATE_270] = { UL, UR, LR, LL },
        };

        vec2 uv_seq[VERTICES_QUAD];
        vertex_attr *v = face->vertices;

        if (!blk_attr->texel.textured)
                return;

        memcpy(uv_seq, blk_attr->texel.uv[face_idx], sizeof(uv_seq));

        for (int i = 0; i < NR_TEXEL_CORNER; ++i) {
                int j = rotated_seq[rotation][i];  // Pick vertex by rotation
                memcpy(v[j].uv, uv_seq[i], sizeof(vec2));
        }

        // Duplicate vertices
        memcpy(v[UR1].uv, v[UR].uv, sizeof(vec2));
        memcpy(v[LL1].uv, v[LL].uv, sizeof(vec2));
}

void block_model_face_vertex_normal(block_face *face, const vec3 origin_gl)
{
        vec3 surround_normals[3];
        vertex_attr *v = face->vertices;

        for (int i = 0; i < VERTICES_TRIANGULATE_QUAD; ++i) {
                if ((v[i].position[X] - origin_gl[X]) > 0) {
                        glm_vec_copy(cube_normals[CUBE_RIGHT], surround_normals[0]);
                } else {
                        glm_vec_copy(cube_normals[CUBE_LEFT], surround_normals[0]);
                }

                if ((v[i].position[Y] - origin_gl[Y]) > 0) {
                        glm_vec_copy(cube_normals[CUBE_TOP], surround_normals[1]);
                } else {
                        glm_vec_copy(cube_normals[CUBE_BOTTOM], surround_normals[1]);
                }

                if ((v[i].position[Z] - origin_gl[Z]) > 0) {
                        glm_vec_copy(cube_normals[CUBE_FRONT], surround_normals[2]);
                } else {
                        glm_vec_copy(cube_normals[CUBE_BACK], surround_normals[2]);
                }

                for (uint32_t j = 0; j < ARRAY_SIZE(surround_normals); ++j) {
                        v[i].normal[X] += surround_normals[j][X];
                        v[i].normal[Y] += surround_normals[j][Y];
                        v[i].normal[Z] += surround_normals[j][Z];
                }
        }
}

int block_model_face_generate(block_model *model, block_attr *blk_attr)
{
        if (!model || !blk_attr)
                return -EINVAL;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_face *face = &(model->faces[i]);

                if (!face->visible)
                        continue;

                block_model_face_vertex(face, blk_attr, model->origin_gl, i);
                block_model_face_vertex_normal(face, model->origin_gl);
                block_model_face_uv(face, blk_attr, i);
        }

        return 0;
}

int block_model_init(block_model *model, vec3 origin_gl)
{
        if (!model)
                return -EINVAL;

        memcpy(model->origin_gl, origin_gl, sizeof(vec3));

        return 0;
}

int block_model_deinit(block_model *model)
{
        if (!model)
                return -EINVAL;

        block_model_face_deinit(model);

        return 0;
}
