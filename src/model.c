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
#include "chunks.h"

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

// Normalized, not rotated
static vec3 cube_normals[] = {
        [CUBE_FRONT]    = {  0.0f,  0.0f,  1.0f },
        [CUBE_BACK]     = {  0.0f,  0.0f, -1.0f },
        [CUBE_TOP]      = {  0.0f,  1.0f,  0.0f },
        [CUBE_BOTTOM]   = {  0.0f, -1.0f,  0.0f },
        [CUBE_LEFT]     = { -1.0f,  0.0f,  0.0f },
        [CUBE_RIGHT]    = {  1.0f,  0.0f,  0.0f },
};

int block_model_face_init(block_face *f)
{
        size_t face_vertices = VERTICES_TRIANGULATE_QUAD;

        if (!f)
                return -EINVAL;

        if (!f->visible)
                return 0;

        f->vertices = memalloc(sizeof(vertex_attr) * face_vertices);
        if (!f->vertices) {
                pr_err_alloc();
                return -ENOMEM;
        }

        return 0;
}

int block_model_face_deinit(block_face *f)
{
        if (!f)
                return -EINVAL;

        if (f->vertices)
                memfree((void **)&f->vertices);

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

static void block_model_face_vertex_uv(block_face *face, block_attr *blk_attr,
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

static inline void block_model_face_normal(block_face *face, int idx)
{
        face->normal[X] = cube_normals[idx][X];
        face->normal[Y] = cube_normals[idx][Y];
        face->normal[Z] = cube_normals[idx][Z];
}

/**
 * block_model_face_generate() - generate single block face vertices
 *
 * @param f: pointer to face
 * @param m: pointer to model
 * @param attr: pointer to block attribute
 * @param idx: face index
 * @return 0 on success
 */
int block_model_face_generate(block_face *f, block_model *m,
                              block_attr *attr, int idx)
{
        if (!f || !m || !attr)
                return -EINVAL;

        if (!f->visible)
                return 0;

        if (!f->vertices)
                return -ENODATA;

        block_model_face_vertex(f, attr, m->origin_gl, idx);
        block_model_face_vertex_normal(f, m->origin_gl);
        block_model_face_vertex_uv(f, attr, idx);

        block_model_face_normal(f, idx);

        return 0;
}

int block_model_generate(block_model *model, block_attr *blk_attr)
{
        if (!model || !blk_attr)
                return -EINVAL;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_face *face = &(model->faces[i]);

                if (block_model_face_generate(face, model, blk_attr, i)) {
                        pr_err_func("block model (%f, %f, %f) face %d failed\n",
                                    model->origin_gl[X], model->origin_gl[Y],
                                    model->origin_gl[Z], i);
                }
        }

        return 0;
}

int block_model_init(block_model *model, vec3 origin_gl)
{
        if (!model)
                return -EINVAL;

        memzero(model, sizeof(block_model));

        memcpy(model->origin_gl, origin_gl, sizeof(vec3));

        return 0;
}

int block_model_deinit(block_model *model)
{
        if (!model)
                return -EINVAL;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_face *f = &(model->faces[i]);
                block_model_face_deinit(f);
        }

        return 0;
}

static inline void block_wireframe_line(line_vec3 line, vec3 a, vec3 b)
{
        glm_vec_copy(a, line[V1]);
        glm_vec_copy(b, line[V2]);
}

void block_wireframe_generate(line_vec3 *line, float scale, const vec3 origin_gl)
{
        vec3 cube_vertices[VERTICES_CUBE];
        float l_2 = BLOCK_EDGE_LEN_GLUNIT / 2.0f * scale;

        ivec2 cube_line_pairs[LINES_CUBE] = {
                { V1, V2 }, { V2, V4 }, { V4, V3 }, { V3, V1 },
                { V5, V6 }, { V6, V8 }, { V8, V7 }, { V7, V5 },
                { V1, V5 }, { V2, V6 }, { V3, V7 }, { V4, V8 },
        };

        // Top face
        cube_vertices[V1][X] = origin_gl[X] - l_2;
        cube_vertices[V1][Y] = origin_gl[Y] + l_2;
        cube_vertices[V1][Z] = origin_gl[Z] - l_2;

        cube_vertices[V2][X] = origin_gl[X] + l_2;
        cube_vertices[V2][Y] = origin_gl[Y] + l_2;
        cube_vertices[V2][Z] = origin_gl[Z] - l_2;

        cube_vertices[V3][X] = origin_gl[X] - l_2;
        cube_vertices[V3][Y] = origin_gl[Y] + l_2;
        cube_vertices[V3][Z] = origin_gl[Z] + l_2;

        cube_vertices[V4][X] = origin_gl[X] + l_2;
        cube_vertices[V4][Y] = origin_gl[Y] + l_2;
        cube_vertices[V4][Z] = origin_gl[Z] + l_2;

        // Bottom face
        cube_vertices[V5][X] = origin_gl[X] - l_2;
        cube_vertices[V5][Y] = origin_gl[Y] - l_2;
        cube_vertices[V5][Z] = origin_gl[Z] - l_2;

        cube_vertices[V6][X] = origin_gl[X] + l_2;
        cube_vertices[V6][Y] = origin_gl[Y] - l_2;
        cube_vertices[V6][Z] = origin_gl[Z] - l_2;

        cube_vertices[V7][X] = origin_gl[X] - l_2;
        cube_vertices[V7][Y] = origin_gl[Y] - l_2;
        cube_vertices[V7][Z] = origin_gl[Z] + l_2;

        cube_vertices[V8][X] = origin_gl[X] + l_2;
        cube_vertices[V8][Y] = origin_gl[Y] - l_2;
        cube_vertices[V8][Z] = origin_gl[Z] + l_2;

        for (int i = 0; i < LINES_CUBE; ++i) {
                block_wireframe_line(line[i],
                                     cube_vertices[cube_line_pairs[i][0]],
                                     cube_vertices[cube_line_pairs[i][1]]);
        }
}

int block_wireframe_draw(ivec3 origin_l, vec4 color, int invert_color, mat4 mat_transform)
{
        float wireframe_scale = 1.005f; // To make a little outside
        line_vec3 lines[LINES_CUBE];
        vec3 origin_gl = { 0 };

        point_local_to_gl(origin_l, BLOCK_EDGE_LEN_GLUNIT, origin_gl);

        block_wireframe_generate(lines, wireframe_scale, origin_gl);

        line_draw(&lines[0][0][0], VERTICES_LINE * LINES_CUBE,
                  color, invert_color ? GL_INVERT : GL_COPY, mat_transform);

        return 0;
}
