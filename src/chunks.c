#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <GL/glew.h>

#include "debug.h"
#include "utils.h"
#include "block.h"
#include "model.h"
#include "glutils.h"
#include "chunks.h"

static inline float __local_to_gl(int x, int l)
{
        int coef = (x < 0) ? (-1) : (1);

        return (coef * l * (abs(x) + (1.0f / 2.0f)));
}

/**
 * coordinate_local_to_gl() - convert block/chunk local space to gl space
 *
 *                |
 *                |
 *                |   <- GL
 *                |
 *       GL(0, 0) |
 *  --------------+--------------> +x
 *      |         |  (0, 0) |
 *      |    .    |    .    |   <- Block/Chunk
 *      | (-1, 0) |         |
 *      |---------|---------|
 *                |     ^
 *                | edge len
 *                |
 *                v
 *                +z
 *
 * @param local: local coordinate
 * @param edge_len: cube edge length
 * @param gl: output gl coordinate
 */
void coordinate_local_to_gl(const ivec3 local, int edge_len, vec3 gl)
{
        gl[X] = __local_to_gl(local[X], edge_len);
        gl[Y] = __local_to_gl(local[Y], edge_len);
        gl[Z] = __local_to_gl(local[Z], edge_len);
}

/**
 * block_init() - init a block
 *
 * @param b: pointer to block variable
 * @param blk_attr: pointer to block attribute
 * @param origin_block: block local origin
 * @return 0 on success
 */
int block_init(block *b, block_attr *blk_attr, ivec3 origin_block)
{
        if (!b || !blk_attr)
                return -EINVAL;

        memzero(b, sizeof(block));

        memcpy(b->origin_l, origin_block, sizeof(ivec3));
        b->blk_attr = blk_attr;

        /* TODO: axis */

        return 0;
}

int block_deinit(block *b)
{
        if (!b)
                return -EINVAL;

        if (b->model) {
                block_model_deinit(b->model);
                free(b->model);
        }

        return 0;
}

static inline int __block_in_chunk(int x, int stride)
{
        return (x / stride);
}

/**
 * block_in_chunk() - get chunk local origin by block local origin
 *
 * @param origin_block: block local origin
 * @param chunk_length: chunk edge length
 * @param origin_chunk: result chunk local origin
 * @return 0 on success
 */
int block_in_chunk(ivec3 origin_block, int chunk_length, ivec3 origin_chunk)
{
        int stride = (chunk_length / BLOCK_EDGE_LEN_GLUNIT);

        origin_chunk[X] = __block_in_chunk(origin_block[X], stride);
        origin_chunk[Y] = __block_in_chunk(origin_block[Y], stride);
        origin_chunk[Z] = __block_in_chunk(origin_block[Z], stride);

        return 0;
}

int block_generate_model(block *b)
{
        vec3 origin_gl = { 0 };

        if (b->model)
                pr_err_func("block model heap is corrupted or already generated\n");

        b->model = memalloc(sizeof(block_model));
        if (!b->model)
                return -ENOMEM;

        coordinate_local_to_gl(b->origin_l, BLOCK_EDGE_LEN_GLUNIT, origin_gl);

        block_model_init(b->model, origin_gl);
        block_model_faces_alloc(b->model);
        block_model_faces_generate(b->model, b->blk_attr);

        return 0;
}

int block_draw(block *b, mat4 mat_transform, int vbo_enabled)
{
        if (!b)
                return -EINVAL;

        if (!b->blk_attr->visible)
                return 0;

        if (b->model == NULL) {
                block_generate_model(b);

                if (!vbo_enabled)
                        block_model_gl_attr(b->model, b->blk_attr);
                else
                        block_model_gl_attr_vbo(b->model, b->blk_attr);

                block_model_faces_free(b->model);
        }

        if (!vbo_enabled)
                block_model_draw(b->model, mat_transform);
        else
                block_model_draw_indexed(b->model, mat_transform);

        return 0;
}

int chunk_init(chunk *c, ivec3 origin_chunk)
{
        if (!c)
                return -EINVAL;

        memzero(c, sizeof(chunk));

        memcpy(c->origin_l, origin_chunk, sizeof(ivec3));

        linklist_alloc(&c->blocks);
        linklist_init(c->blocks, sizeof(block));

        seqlist_alloc(&c->vertices);
        seqlist_init(c->vertices, sizeof(vertex_attr), 128);

        c->state = CHUNK_INITED;

        return 0;
}

int chunk_deinit(chunk *c)
{
        linklist_node *pos;

        if (!c)
                return -EINVAL;

        linklist_for_each_node(pos, c->blocks->head) {
                block_deinit(pos->data);
        }

        linklist_deinit(c->blocks);
        linklist_free(&c->blocks);

        seqlist_deinit(c->vertices);
        seqlist_free(&c->vertices);

        memzero(c, sizeof(chunk));

        return 0;
}

linklist_node *chunk_get_block_node(chunk *c, ivec3 origin_block)
{
        linklist_node *pos;
        linklist_node *ret = NULL;

        if (!c)
                return NULL;

        linklist_for_each_node(pos, c->blocks->head) {
                block *b = pos->data;

                if (ivec3_equal(b->origin_l, origin_block)) {
                        ret = pos;
                        break;
                }
        }

        return ret;
}

block *chunk_get_block(chunk *c, ivec3 origin_block)
{
        linklist_node *node;

        node = chunk_get_block_node(c, origin_block);
        if (node == NULL) {
                pr_err_func("block (%d, %d, %d) not found in chunk (%d, %d, %d)\n",
                            origin_block[X], origin_block[Y], origin_block[Z],
                            c->origin_l[X], c->origin_l[Y], c->origin_l[Z]);
                return NULL;
        }

        return node->data;
}

block *chunk_add_block(chunk *c, block *b)
{
        block *ret;

        if (!c)
                return NULL;

        ret = linklist_append(c->blocks, b);
        if (ret == NULL)
                pr_err_func("linklist_append() failed\n");

        c->state = CHUNK_NEED_UPDATE;

        return ret;
}

int chunk_del_block(chunk *c, ivec3 origin_block)
{
        linklist_node *node;

        if (!c)
                return -EINVAL;

        node = chunk_get_block_node(c, origin_block);
        if (node == NULL) {
                pr_err_func("block (%d, %d, %d) not found in chunk (%d, %d, %d)\n",
                            origin_block[X], origin_block[Y], origin_block[Z],
                            c->origin_l[X], c->origin_l[Y], c->origin_l[Z]);
                return -ENODATA;
        }

        block_deinit(node->data);
        linklist_delete(c->blocks, node);

        c->state = CHUNK_NEED_UPDATE;

        return 0;
}

int chunk_vertices_pack(chunk *c)
{
        linklist_node *pos;

        if (!c)
                return 0;

        linklist_for_each_node(pos, c->blocks->head) {
                block *b = pos->data;

                block_generate_model(b);

                for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                        block_face *f = &(b->model->faces[i]);

                        for (int j = 0; j < VERTICES_TRIANGULATE_QUAD; ++j) {
                                seqlist_append(c->vertices, &f->vertex[j]);
                        }
                }
        }

        seqlist_shrink(c->vertices);

        return 0;
}

int chunk_vertices_free(chunk *c)
{
        if (!c)
                return -EINVAL;

        return seqlist_deinit(c->vertices);
}

void chunk_cleanup(chunk *c)
{
        if (glIsBuffer(c->glattr.vertex) != GL_FALSE)
                gl_attr_buffer_delete(&c->glattr);

        memzero(&c->glattr, sizeof(gl_attr));
}

int chunk_gl_attr_generate(gl_attr *glattr, block_attr *blk_dummy)
{
        int ret;

        if (!glattr)
                return -EINVAL;

        glattr->program = block_shader_get(blk_dummy->shader);
        ret  = glIsProgram(glattr->program);
        if (ret == GL_FALSE) {
                pr_err_func("invalid shader program\n");
                return -EFAULT;
        }

        ret = glIsTexture(blk_dummy->texel.texel);
        if (ret == GL_FALSE) {
                pr_err_func("invalid texture object\n");
                return -EFAULT;
        } else {
                glattr->texel = blk_dummy->texel.texel;
        }

        glattr->sampler = glGetUniformLocation(glattr->program, "sampler");
        glattr->mat_transform = glGetUniformLocation(glattr->program, "mat_transform");

        return 0;
}

int chunk_gl_vbo_buffer_generate(chunk *c)
{
        gl_vbo vbo;
        int ret;

        ret = gl_vbo_init(&vbo);
        if (ret)
                return -ENOMEM;

        gl_vbo_index(&vbo, c->vertices->data,
                     (uint32_t)c->vertices->count_utilized);

        ret = gl_vbo_buffer_create(&vbo, &c->glattr);
        if (ret == GL_FALSE)
                pr_err_func("failed to generate chunk VBO\n");

        gl_vbo_deinit(&vbo);

        return ret;
}

int chunk_gl_data_update(chunk *c)
{
        if (!c)
                return -EINVAL;

        c->state = CHUNK_UPDATING;

        chunk_cleanup(c);

        chunk_vertices_pack(c);

        chunk_gl_attr_generate(&c->glattr, block_attr_get(BLOCK_DUMMY));
        chunk_gl_vbo_buffer_generate(c);

        chunk_vertices_free(c);

        c->state = CHUNK_UPDATED;

        return 0;
}

int chunk_draw(chunk *c, mat4 mat_transform)
{
        gl_attr *glattr;

        if (!c)
                return -EINVAL;

        if (c->state != CHUNK_UPDATED)
                return -EBUSY;

        glattr = &c->glattr;

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

        return 0;
}

int world_init(world *w)
{
        if (!w)
                return -EINVAL;

        memzero(w, sizeof(world));

        w->chunk_length = CHUNK_EDGE_LEN_GLUNIT;
        w->height_max = WORLD_HEIGHT_MAX; /* -1 */
        w->height_min = WORLD_HEIGHT_MIN;

        linklist_alloc(&w->chunks);
        linklist_init(w->chunks, sizeof(chunk));

        return 0;
}

int world_deinit(world *w)
{
        linklist_node *pos;

        if (!w)
                return -EINVAL;

        linklist_for_each_node(pos, w->chunks->head) {
                chunk_deinit(pos->data);
        }

        linklist_deinit(w->chunks);
        linklist_free(&w->chunks);

        return 0;
}

chunk *world_add_chunk(world *w, ivec3 origin_chunk)
{
        chunk c;
        chunk *ret;

        chunk_init(&c, origin_chunk);

        ret = linklist_append(w->chunks, &c);
        if (ret == NULL)
                pr_err_func("linklist_append() failed\n");

        return ret;
}

chunk *world_get_chunk(world *w, ivec3 origin_chunk)
{
        chunk *c;
        linklist_node *pos;

        if (linklist_is_empty(w->chunks))
                return NULL;

        linklist_for_each_node(pos, w->chunks->head) {
                c = pos->data;

                if (ivec3_equal(origin_chunk, c->origin_l))
                        return c;
        }

        return NULL;
}

int world_add_block(world *w, block *b)
{
        ivec3 origin_chunk = { 0 };
        chunk *c;

        if (!w || !b)
                return -EINVAL;

        if (b->origin_l[Y] < w->height_min || b->origin_l[Y] > w->height_max) {
                pr_info_func("block height exceeds limits [%d - %d]\n",
                             w->height_min, w->height_max);
                return -EINVAL;
        }

        block_in_chunk(b->origin_l, w->chunk_length, origin_chunk);

        c = world_get_chunk(w, origin_chunk);
        if (c == NULL)
                c = world_add_chunk(w, origin_chunk);

        chunk_add_block(c, b);

        // TODO: Check whether block should be draw, if so generate model

        return 0;
}

int world_del_block(world *w, ivec3 origin_block)
{
        ivec3 origin_chunk = { 0 };
        chunk *c;

        if (!w)
                return -EINVAL;

        block_in_chunk(origin_block, w->chunk_length, origin_chunk);

        c = world_get_chunk(w, origin_chunk);
        if (c == NULL) {
                pr_err_func("internal error: chunk not found\n");
                return -EFAULT;
        }

        chunk_del_block(c, origin_block);

        return 0;
}

// TODO: Render visible block only (Important optimization)
int chunk_draw_block(chunk *c, mat4 mat_transform)
{
        linklist_node *pos;

        if (!c)
                return -EINVAL;

        linklist_for_each_node(pos, c->blocks->head) {
                block_draw(pos->data, mat_transform, GL_VBO_ENABLED);
        }


        return 0;
}

int world_draw_block(world *w, mat4 mat_transform /* TODO: Render distance */)
{
        linklist_node *pos;

        if (!w)
                return -EINVAL;

        linklist_for_each_node(pos, w->chunks->head) {
                chunk_draw_block(pos->data, mat_transform);
        }

        return 0;
}

int world_draw_chunk(world *w, mat4 mat_transform)
{
        linklist_node *pos;

        if (!w)
                return -EINVAL;

        glEnable(GL_CULL_FACE);

        linklist_for_each_node(pos, w->chunks->head) {
                chunk_draw(pos->data, mat_transform);
        }

        glDisable(GL_CULL_FACE);

        return 0;
}

int world_update_chunk(world *w)
{
        linklist_node *pos;

        if (!w)
                return -EINVAL;

        linklist_for_each_node(pos, w->chunks->head) {
                chunk *c = pos->data;

                if (c->state == CHUNK_NEED_UPDATE)
                        chunk_gl_data_update(c);
        }

        return 0;
}

int block_dump(block *b)
{
        if (!b)
                return -EINVAL;

        pr_info("Block (%d, %d, %d) Type: %s\n",
                b->origin_l[X], b->origin_l[Y], b->origin_l[Z],
                b->blk_attr->name);

        return 0;
}

int chunk_dump(chunk *c)
{
        linklist_node *pos;

        if (!c)
                return -EINVAL;

        pr_info("Chunk (%d, %d, %d): Block Count: %zd\n",
                c->origin_l[X], c->origin_l[Y], c->origin_l[Z],
                c->blocks->element_count);

        linklist_for_each_node(pos, c->blocks->head) {
                block_dump(pos->data);
        }

        return 0;
}

int world_block_dump(world *w)
{
        linklist_node *pos;

        if (!w)
                return -EINVAL;

        pr_info_func("\n");
        pr_info("Height: [%d - %d]\n", w->height_min, w->height_max);
        pr_info("Chunk Length: %d\n", w->chunk_length);
        pr_info("Chunk Count: %zd\n", w->chunks->element_count);

        size_t i = 0;
        linklist_for_each_node(pos, w->chunks->head) {
                pr_info("Chunk [%zd]\n", i);
                chunk_dump(pos->data);
                i++;
        }

        return 0;
}
