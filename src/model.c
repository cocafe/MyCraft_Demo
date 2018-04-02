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

int block_model_init(block_model *mesh, vec3 origin_gl)
{
        if (!mesh)
                return -EINVAL;

        memzero(mesh, sizeof(block_model));

        memcpy(mesh->origin_gl, origin_gl, sizeof(vec3));

        return 0;
}

int block_model_deinit(block_model *mesh)
{
        gl_attr *glattr;

        if (!mesh)
                return -EINVAL;

        glattr = &mesh->glattr;

        if (glIsBuffer(glattr->vertex) != GL_FALSE)
                buffer_delete(&glattr->vertex);

        if (glIsBuffer(glattr->vertex) != GL_FALSE)
                buffer_delete(&glattr->vertex_nrm);

        if (glIsBuffer(glattr->uv) != GL_FALSE)
                buffer_delete(&glattr->uv);

        memzero(mesh, sizeof(block_model));

        return 0;
}


static void block_model_face_vertex(block_face *face, block_attr *blk_attr,
                                    const vec3 origin_gl, int face_idx)
{
        float w = blk_attr->size_model.width;
        float h = blk_attr->size_model.height;
        float l = blk_attr->size_model.length;

        switch (face_idx) {
                case CUBE_FRONT:
                        face->vertex[UL][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_BACK:
                        face->vertex[UL][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        break;

                case CUBE_TOP:
                        face->vertex[UL][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_BOTTOM:
                        face->vertex[UL][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        break;

                case CUBE_LEFT:
                        face->vertex[UL][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin_gl[X] - (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        break;

                case CUBE_RIGHT:
                        face->vertex[UL][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[UL][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UL][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[UR][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[UR][Y] = origin_gl[Y] + (1.0f / 2.0f) * h;
                        face->vertex[UR][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        face->vertex[LL][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[LL][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LL][Z] = origin_gl[Z] + (1.0f / 2.0f) * l;

                        face->vertex[LR][X] = origin_gl[X] + (1.0f / 2.0f) * w;
                        face->vertex[LR][Y] = origin_gl[Y] - (1.0f / 2.0f) * h;
                        face->vertex[LR][Z] = origin_gl[Z] - (1.0f / 2.0f) * l;

                        break;

                default:
                        break;
        }

        // Duplicate vertices
        memcpy(face->vertex[UR1], face->vertex[UR], sizeof(vec3));
        memcpy(face->vertex[LL1], face->vertex[LL], sizeof(vec3));
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

        if (!blk_attr->texel.textured)
                return;

        memcpy(uv_seq, blk_attr->texel.uv[face_idx], sizeof(uv_seq));

        for (int i = 0; i < NR_TEXEL_CORNER; ++i) {
                int j = rotated_seq[rotation][i];  // Pick vertex by rotation
                memcpy(face->uv[j], uv_seq[i], sizeof(vec2));
        }

        // Duplicate vertices
        memcpy(face->uv[UR1], face->uv[UR], sizeof(vec2));
        memcpy(face->uv[LL1], face->uv[LL], sizeof(vec2));
}

void block_model_face_vertex_normal(block_face *face, const vec3 origin_gl)
{
        vec3 surround_normals[3];

        for (int i = 0; i < VERTICES_TRIANGULATE_QUAD; ++i) {
                if ((face->vertex[i][X] - origin_gl[X]) > 0) {
                        glm_vec_copy(cube_normals[CUBE_RIGHT], surround_normals[0]);
                } else {
                        glm_vec_copy(cube_normals[CUBE_LEFT], surround_normals[0]);
                }

                if ((face->vertex[i][Y] - origin_gl[Y]) > 0) {
                        glm_vec_copy(cube_normals[CUBE_TOP], surround_normals[1]);
                } else {
                        glm_vec_copy(cube_normals[CUBE_BOTTOM], surround_normals[1]);
                }

                if ((face->vertex[i][Z] - origin_gl[Z]) > 0) {
                        glm_vec_copy(cube_normals[CUBE_FRONT], surround_normals[2]);
                } else {
                        glm_vec_copy(cube_normals[CUBE_BACK], surround_normals[2]);
                }

                for (uint32_t j = 0; j < ARRAY_SIZE(surround_normals); ++j) {
                        face->vertex_normal[i][X] += surround_normals[j][X];
                        face->vertex_normal[i][Y] += surround_normals[j][Y];
                        face->vertex_normal[i][Z] += surround_normals[j][Z];
                }
        }
}

int block_model_gl_attr(block_model *mesh, block_face *faces, block_attr *blk_attr)
{
        gl_attr *glattr = &mesh->glattr;
        size_t  vertex_count = VERTICES_TRIANGULATE_CUBE;
        int     ret = GL_FALSE;
        float   *buf_vertex_nrm;
        float   *buf_vertex;
        float   *buf_uv;

        buf_vertex_nrm = memalloc(sizeof(vec3) * vertex_count);
        if (!buf_vertex_nrm) {
                pr_err_alloc();
                return -ENOMEM;
        }

        buf_vertex = memalloc(sizeof(vec3) * vertex_count);
        if (!buf_vertex) {
                pr_err_alloc();
                goto free_vert_nrm;
        }

        buf_uv = memalloc(sizeof(vec3) * vertex_count);
        if (!buf_uv) {
                pr_err_alloc();
                goto free_vert;
        }

        for (int i = 0; i < CUBE_QUAD_FACES; i++) {
                block_face *face = &faces[i];

                memcpy(&buf_vertex_nrm[i * VERTICES_TRIANGULATE_QUAD * 3],
                       face->vertex_normal,
                       sizeof(face->vertex_normal));

                memcpy(&buf_vertex[i * VERTICES_TRIANGULATE_QUAD * 3],
                       face->vertex,
                       sizeof(face->vertex));

                mempcpy(&buf_uv[i * VERTICES_TRIANGULATE_QUAD * 2],
                        face->uv,
                        sizeof(face->uv));
        }

        glattr->vertex = buffer_create(buf_vertex, sizeof(vec3) * vertex_count);
        ret = glIsBuffer(glattr->vertex);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex buffer\n");
                goto free_alloc;
        }

        glattr->vertex_nrm = buffer_create(buf_vertex_nrm, sizeof(vec3) * vertex_count);
        ret = glIsBuffer(glattr->vertex);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex normal buffer\n");
                goto err_vert_nrm;
        }

        glattr->uv = buffer_create(buf_uv, sizeof(vec2) * vertex_count);
        ret = glIsBuffer(glattr->uv);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex normal buffer\n");
                goto err_uv;
        }

        glattr->program = block_shader_get(blk_attr->shader);
        ret = glIsProgram(glattr->program);
        if (ret == GL_FALSE) {
                pr_err_func("invalid block shader program\n");
                goto err_program;
        }

        if (blk_attr->texel.textured) {
                ret = glIsTexture(blk_attr->texel.texel);
                if (ret == GL_FALSE) {
                        pr_err_func("invalid texture object\n");
                        goto err_texture;
                }

                glattr->texel = blk_attr->texel.texel;
        }

        glattr->sampler = glGetUniformLocation(glattr->program, "sampler");
        glattr->mat_transform = glGetUniformLocation(glattr->program, "mat_transform");

free_alloc:
        memfree((void **)&buf_uv);

free_vert:
        memfree((void **)&buf_vertex_nrm);

free_vert_nrm:
        memfree((void **)&buf_vertex);

        return ret;

err_texture:
err_program:
        buffer_delete(&glattr->uv);

err_uv:
        buffer_delete(&glattr->vertex_nrm);

err_vert_nrm:
        buffer_delete(&glattr->vertex);

        goto free_alloc;
}

int block_model_generate(block_model *mesh, block_attr *blk_attr)
{
        block_face faces[CUBE_QUAD_FACES];

        memset(faces, 0x00, sizeof(faces));

        if (!mesh || !blk_attr)
                return -EINVAL;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_model_face_vertex(&faces[i], blk_attr, mesh->origin_gl, i);
                block_model_face_vertex_normal(&faces[i], mesh->origin_gl);
                block_model_face_uv(&faces[i], blk_attr, i);
        }

        // TODO: Rotate mesh upon to axis
        block_model_gl_attr(mesh, faces, blk_attr);

        return 0;
}

int block_model_draw(block_model *mesh, mat4 mat_transform)
{
        gl_attr *glattr;

        if (!mesh)
                return -EINVAL;

        glattr = &mesh->glattr;

        glEnable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(GL_PROGRAM_NONE);

        glUseProgram(glattr->program);

        glUniformMatrix4fv(glattr->mat_transform, 1, GL_FALSE, &mat_transform[0][0]);

        if (glattr->texel != GL_TEXTURE_NONE) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, glattr->texel);
                glUniform1i(glattr->sampler, 0);
        }

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, glattr->vertex);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, glattr->vertex_nrm);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, glattr->uv);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)VERTICES_TRIANGULATE_CUBE);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glUseProgram(GL_PROGRAM_NONE);

        glDisable(GL_CULL_FACE);

        return 0;
}
