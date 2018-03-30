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
        UPPER_LEFT = 0,
        UPPER_RIGHT,
        LOWER_LEFT,
        LOWER_RIGHT,
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

static void model_cube_face_vertex(cube_face *face, block *block,
                                   const vec3 origin, int idx)
{
        float w = block->size_model.width;
        float h = block->size_model.height;
        float l = block->size_model.length;

        switch (idx) {
                case CUBE_FRONT:
                        face->vertex[UL][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_BACK:
                        face->vertex[UL][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        break;

                case CUBE_TOP:
                        face->vertex[UL][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_BOTTOM:
                        face->vertex[UL][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        break;

                case CUBE_LEFT:
                        face->vertex[UL][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin[X] - (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_RIGHT:
                        face->vertex[UL][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin[X] + (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin[Z] - (1.0f / 2.0f) * l;

                        break;

                default:
                        break;
        }

        // Duplicate vertices
        memcpy(face->vertex[UR1], face->vertex[UR], sizeof(vec3));
        memcpy(face->vertex[LL1], face->vertex[LL], sizeof(vec3));
}

static void model_cube_face_uv(cube_face *face, block *block, int idx)
{
        int rotate = block->texels[idx].rotation;
        int rotate_seq[][4] = {
                [TEXEL_ROTATE_0]   = { UL, UR, LL, LR },
                [TEXEL_ROTATE_90]  = { LL, UL, LR, UR },
                [TEXEL_ROTATE_180] = { LR, LL, UR, UL },
                [TEXEL_ROTATE_270] = { UR, LR, UL, LL },
        };
        vec2 uv_seq[] = {        /* U , V */
                [UPPER_LEFT]  = { 0.0f, 1.0f },
                [UPPER_RIGHT] = { 1.0f, 1.0f },
                [LOWER_LEFT]  = { 0.0f, 0.0f },
                [LOWER_RIGHT] = { 1.0f, 0.0f },
        };

        if (!block->have_texel)
                return;

        for (int i = 0; i < NR_TEXEL_CORNER; ++i) {
                int j = rotate_seq[rotate][i];  // Pick vertex to compute
                memcpy(face->uv[j], uv_seq[i], sizeof(vec2));
        }

        // Duplicate vertices
        memcpy(face->uv[UR1], face->uv[UR], sizeof(vec2));
        memcpy(face->uv[LL1], face->uv[LL], sizeof(vec2));
}

static int model_cube_face_glattr(cube_face *face, block *block, int idx)
{
        gl_attr *glattr = &face->glattr;
        int ret;

        glattr->vertex = buffer_create(&face->vertex[0][0], sizeof(face->vertex));
        ret = glIsBuffer(glattr->vertex);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex buffer\n");
                goto err_vert;
        }

        glattr->uv = buffer_create(&face->uv[0][0], sizeof(face->uv));
        ret = glIsBuffer(glattr->uv);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create uv buffer\n");
                goto err_uv;
        }

        if (block->have_texel) {
                ret = glIsTexture(block->texels[idx].texture);
                if (ret == GL_FALSE) {
                        pr_err_func("failed to bind texture buffer\n");
                        goto err_texel;
                }

                glattr->texel = block->texels[idx].texture;
        }

        glattr->program = block_shader_get(block->shader);
        ret = glIsProgram(glattr->program);
        if (ret == GL_FALSE) {
                pr_err_func("invalid block shader program\n");
                goto err_program;
        }

        glattr->sampler = glGetUniformLocation(glattr->program, "sampler");
        glattr->mat_transform = glGetUniformLocation(glattr->program, "mat_transform");

        return ret;

err_program:
err_texel:
err_uv:
        buffer_delete(&glattr->vertex);
err_vert:
        return ret;
}

void model_cube_face_normal(cube_face *face, const vec3 origin, int idx)
{
        vec3 normals[] = {
                [CUBE_FRONT]    = {  0.0f,  0.0f,  1.0f },
                [CUBE_BACK]     = {  0.0f,  0.0f, -1.0f },
                [CUBE_TOP]      = {  0.0f,  1.0f,  0.0f },
                [CUBE_BOTTOM]   = {  0.0f, -1.0f,  0.0f },
                [CUBE_LEFT]     = { -1.0f,  0.0f,  0.0f },
                [CUBE_RIGHT]    = {  1.0f,  0.0f,  0.0f },
        };

        face->normal[X] = origin[X] + normals[idx][X];
        face->normal[Y] = origin[Y] + normals[idx][Y];
        face->normal[Z] = origin[Z] + normals[idx][Z];
}

void model_cube_vertex_normal(model_cube *cube, const vec3 origin)
{
        vec3 face_normals[3];

        for (int i = 0; i < CUBE_FACES_QUADS; ++i) {
                cube_face *face = &(cube->faces[i]);

                for (int j = 0; j < VERTICES_QUAD_FACE; ++j) {
                        if ((face->vertex[j][X] - origin[X]) > 0) {
                                glm_vec_copy(cube->faces[CUBE_RIGHT].normal, face_normals[0]);
                        } else {
                                glm_vec_copy(cube->faces[CUBE_LEFT].normal, face_normals[0]);
                        }

                        if ((face->vertex[j][Y] - origin[Y]) > 0) {
                                glm_vec_copy(cube->faces[CUBE_TOP].normal, face_normals[1]);
                        } else {
                                glm_vec_copy(cube->faces[CUBE_BOTTOM].normal, face_normals[1]);
                        }

                        if ((face->vertex[j][Z] - origin[Z]) > 0) {
                                glm_vec_copy(cube->faces[CUBE_FRONT].normal, face_normals[2]);
                        } else {
                                glm_vec_copy(cube->faces[CUBE_BACK].normal, face_normals[2]);
                        }

                        // Vertex normal:
                        // Combination of the normals of surrounding faces
                        for (uint32_t k = 0; k < ARRAY_SIZE(face_normals); ++k) {
                                face->vertex_normal[j][X] += face_normals[k][X];
                                face->vertex_normal[j][Y] += face_normals[k][Y];
                                face->vertex_normal[j][Z] += face_normals[k][Z];
                        }
                }
        }
}

void model_cube_axis_set(model_cube *cube)
{
        memcpy(cube->axis[X], cube->faces[CUBE_RIGHT].normal, sizeof(vec3));
        memcpy(cube->axis[Y], cube->faces[CUBE_TOP].normal, sizeof(vec3));
        memcpy(cube->axis[Z], cube->faces[CUBE_FRONT].normal, sizeof(vec3));
}

int model_cube_generate(model_cube *cube, block *block, vec3 origin /* front vector*/)
{
        if (!cube || !block)
                return -EINVAL;

        // Clean memory first, so that we can leave
        // it blank if we don't need data in there
        memzero(cube, sizeof(model_cube));

        cube->block = block;
        glm_vec_copy(origin, cube->origin);

        // We don't need to prepare data and draw for invisible block
        if (!block->visible)
                return 0;

        for (int i = 0; i < CUBE_FACES_QUADS; ++i) {
                cube_face *face = &(cube->faces[i]);

                model_cube_face_vertex(face, block, origin, i);
                model_cube_face_uv(face, block, i);
                model_cube_face_normal(face, origin, i);

                if (block->visible)
                        model_cube_face_glattr(face, block, i);
        }

        model_cube_vertex_normal(cube, origin);
        // TODO: Rotate axis and cube by player front vector
        model_cube_axis_set(cube);

        return 0;
}

int model_cube_delete(model_cube *cube)
{
        if (!cube)
                return -EINVAL;

        // Clean allocated glBuffers
        if (cube->block->visible) {
                for (int i = 0; i < CUBE_FACES_QUADS; ++i) {
                        gl_attr *glattr = &(cube->faces[i].glattr);

                        // Shader programs are created in somewhere else
                        glattr->program = GL_PROGRAM_NONE;
                        // Texture are create in somewhere else
                        glattr->texel = GL_TEXTURE_NONE;

                        if (glIsBuffer(glattr->vertex) != GL_FALSE)
                                buffer_delete(&glattr->vertex);

                        if (glIsBuffer(glattr->uv) != GL_FALSE)
                                buffer_delete(&glattr->uv);
                }
        }

        memzero(cube, sizeof(model_cube));

        return 0;
}

int model_cube_draw(model_cube *cube, mat4 mat_transform)
{
        if (!cube)
                return -EINVAL;

        if (!cube->block->visible)
                return 0;

        glEnable(GL_CULL_FACE);

//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(GL_PROGRAM_NONE);

        for (int i = 0; i < CUBE_FACES_QUADS; ++i) {
                cube_face *face = &(cube->faces[i]);
                gl_attr *glattr = &face->glattr;

                glUseProgram(glattr->program);

                glUniformMatrix4fv(glattr->mat_transform, 1, GL_FALSE, &mat_transform[0][0]);

                if (cube->block->have_texel) {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, glattr->texel);
                        glUniform1i(glattr->sampler, 0);
                }

                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, glattr->vertex);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

                glEnableVertexAttribArray(1);
                glBindBuffer(GL_ARRAY_BUFFER, glattr->uv);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

                glDrawArrays(GL_TRIANGLES, 0, (GLsizei)VERTICES_QUAD_FACE);

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
        }

        glUseProgram(GL_PROGRAM_NONE);

        glDisable(GL_CULL_FACE);

        return 0;
}
