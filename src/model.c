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

int block_model_faces_alloc(block_model *model)
{
        if (!model)
                return -EINVAL;

        model->faces = memalloc(sizeof(block_face) * CUBE_QUAD_FACES);
        if (!model->faces) {
                pr_err_alloc();
                return -ENOMEM;
        }

        return 0;
}

int block_model_faces_free(block_model *model)
{
        if (!model)
                return -EINVAL;

        memfree((void **)&model->faces);

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

        if (model->faces)
                block_model_faces_free(model);

        if (model->glvbo) {
                gl_vbo_deinit(model->glvbo);
                memfree((void **)&model->glvbo);
        }

        if (model->glattr) {
                gl_attr *glattr = model->glattr;

                gl_attr_buffer_delete(glattr);

                memfree((void **)&model->glattr);
        }

        memzero(model, sizeof(block_model));

        return 0;
}

static void block_model_face_vertex(block_face *face, block_attr *blk_attr,
                                    const vec3 origin_gl, int face_idx)
{
        float w = blk_attr->size_model.width;
        float h = blk_attr->size_model.height;
        float l = blk_attr->size_model.length;
        vertex_attr *v = face->vertex;

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
        vertex_attr *v = face->vertex;

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
        vertex_attr *v = face->vertex;

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

int block_model_faces_generate(block_model *model, block_attr *blk_attr)
{
        block_face *faces;

        if (!model || !blk_attr)
                return -EINVAL;

        if (!model->faces)
                return -ENODATA;

        faces = model->faces;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_model_face_vertex(&faces[i], blk_attr, model->origin_gl, i);
                block_model_face_vertex_normal(&faces[i], model->origin_gl);
                block_model_face_uv(&faces[i], blk_attr, i);
        }

        return 0;
}

int block_model_face_vertices_copy(block_model *model, vec3 *vertices,
                                   vec3 *normals, vec2 *uvs)
{
        if (!model)
                return -EINVAL;

        if (!model->faces)
                return -ENODATA;

        for (int i = 0; i < CUBE_QUAD_FACES; i++) {
                block_face *face = &(model->faces)[i];
                vertex_attr *pack = face->vertex;
                int offset = i * VERTICES_TRIANGULATE_QUAD;

                for (int j = 0; j < VERTICES_TRIANGULATE_QUAD; ++j) {
                        memcpy(&uvs[offset + j], pack[j].uv, sizeof(pack[j].uv));
                        memcpy(&normals[offset + j], pack[j].normal, sizeof(pack[j].normal));
                        memcpy(&vertices[offset + j], pack[j].position, sizeof(pack[j].position));
                }
        }

        return 0;
}

vertex_attr *block_model_vertices_pack(block_model *model)
{
        vertex_attr *vertex_pack;
        size_t vertex_count;

        if (!model)
                return NULL;

        if (!model->faces)
                return NULL;

        vertex_count = VERTICES_TRIANGULATE_CUBE;
        vertex_pack = memalloc(sizeof(vertex_attr) * vertex_count);
        if (!vertex_pack)
                return NULL;

        for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                block_face *face = &model->faces[i];
                int vertex_face = VERTICES_TRIANGULATE_QUAD;

                memcpy(&vertex_pack[i * vertex_face],
                       face->vertex, sizeof(face->vertex));
        }

        return vertex_pack;
}

/**
 * block_model_gl_attr() - reserved for drawing single not indexed block
 *
 * @param model: pointer to block model
 * @param blk_attr: pointer to block style
 * @return 0 on success
 */
int block_model_gl_attr(block_model *model, block_attr *blk_attr)
{
        vec3            *vertices;
        vec3            *normals;
        vec2            *uvs;

        size_t          count = VERTICES_TRIANGULATE_CUBE;
        int             ret;

        if (!model)
                return -EINVAL;

        if (!model->faces)
                return -ENODATA;

        /*
         * Since drawing one block only is an optional
         * feature, in order to reduce memory footprint
         * we make it manually heap allocation instead of
         * on stack.
         */
        if (model->glattr == NULL) {
                model->glattr = memalloc(sizeof(gl_attr));
                if (!model->glattr)
                        return -ENOMEM;
        }

        gl_attr *glattr = model->glattr;

        gl_vertices_alloc(&vertices, &normals, &uvs, count);

        block_model_face_vertices_copy(model, vertices, normals, uvs);

        glattr->vertex = buffer_create(vertices, sizeof(vec3) * count);
        ret = glIsBuffer(glattr->vertex);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex buffer\n");
                goto free_alloc;
        }

        glattr->vertex_nrm = buffer_create(normals, sizeof(vec3) * count);
        ret = glIsBuffer(glattr->vertex);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex normal buffer\n");
                goto del_vertex;
        }

        glattr->vertex_uv = buffer_create(uvs, sizeof(vec2) * count);
        ret = glIsBuffer(glattr->vertex_uv);
        if (ret == GL_FALSE) {
                pr_err_func("failed to create vertex normal buffer\n");
                goto del_vertex_normal;
        }

        glattr->program = block_shader_get(blk_attr->shader);
        ret = glIsProgram(glattr->program);
        if (ret == GL_FALSE) {
                pr_err_func("invalid block shader program\n");
                goto del_uv;
        }

        if (blk_attr->texel.textured) {
                ret = glIsTexture(blk_attr->texel.texel);
                if (ret == GL_FALSE) {
                        pr_err_func("invalid texture object\n");
                        goto free_alloc;
                }

                glattr->texel = blk_attr->texel.texel;
        }

        // Define amount of vertices to draw
        glattr->vertex_count = (GLsizei)count;

        glattr->sampler = glGetUniformLocation(glattr->program, "sampler");
        glattr->mat_transform = glGetUniformLocation(glattr->program, "mat_transform");

free_alloc:
        gl_vertices_free(&vertices, &normals, &uvs);

        return ret;

del_uv:
        buffer_delete(&glattr->vertex_uv);

del_vertex_normal:
        buffer_delete(&glattr->vertex_nrm);

del_vertex:
        buffer_delete(&glattr->vertex);

        goto free_alloc;
}

int block_model_gl_attr_vbo(block_model *model, block_attr *blk_attr)
{
        int ret;

        if (!model)
                return -EINVAL;

        if (!model->faces)
                return -ENODATA;

        /*
         * Since drawing one block only is an optional
         * feature, in order to reduce memory footprint
         * we make it manually heap allocation instead of
         * on stack.
         */
        if (model->glattr == NULL) {
                model->glattr = memalloc(sizeof(gl_attr));
                if (!model->glattr)
                        return -ENOMEM;
        }

        if (model->glvbo == NULL) {
                model->glvbo = memalloc(sizeof(gl_vbo));
                if (!model->glvbo)
                        return -ENOMEM;
        }

        gl_attr *glattr = model->glattr;
        gl_vbo *glvbo = model->glvbo;

        vertex_attr *vertices = block_model_vertices_pack(model);
        uint32_t count = VERTICES_TRIANGULATE_CUBE;

        ret = gl_vbo_init(glvbo);
        if (ret)
                goto free_alloc;

        gl_vbo_index(glvbo, vertices, count);
        ret = gl_vbo_buffer_create(glvbo, glattr);
        if (ret == GL_FALSE)
                goto free_vbo;

        glattr->program = block_shader_get(blk_attr->shader);
        ret = glIsProgram(glattr->program);
        if (ret == GL_FALSE) {
                pr_err_func("invalid block shader program\n");
                goto free_bufs;
        }

        if (blk_attr->texel.textured) {
                ret = glIsTexture(blk_attr->texel.texel);
                if (ret == GL_FALSE) {
                        pr_err_func("invalid texture object\n");
                        goto free_bufs;
                }

                glattr->texel = blk_attr->texel.texel;
        }

        glattr->sampler = glGetUniformLocation(glattr->program, "sampler");
        glattr->mat_transform = glGetUniformLocation(glattr->program, "mat_transform");

free_vbo:
        // OpenGL has our copy now, free memory
        gl_vbo_deinit(glvbo);

free_alloc:
        memfree((void **)&vertices);

        return ret;

free_bufs:
        gl_vbo_buffer_delete(glattr);

        goto free_vbo;
}

int block_model_draw(block_model *model, mat4 mat_transform)
{
        gl_attr *glattr;

        if (!model)
                return -EINVAL;

        glattr = model->glattr;

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
        glBindBuffer(GL_ARRAY_BUFFER, glattr->vertex_uv);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glDrawArrays(GL_TRIANGLES, 0, glattr->vertex_count);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glUseProgram(GL_PROGRAM_NONE);

        glDisable(GL_CULL_FACE);

        return 0;
}

int block_model_draw_indexed(block_model *model, mat4 mat_transform)
{
        gl_attr *glattr;

        if (!model)
                return -EINVAL;

        glattr = model->glattr;

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
        glBindBuffer(GL_ARRAY_BUFFER, glattr->vertex_uv);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glattr->vbo_index);
        glDrawElements(GL_TRIANGLES, glattr->vertex_count, GL_UNSIGNED_INT, (void *)0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glUseProgram(GL_PROGRAM_NONE);

        glDisable(GL_CULL_FACE);

        return 0;
}
