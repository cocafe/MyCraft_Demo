#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>
#include <pthread.h>

#include <GL/glew.h>

#include "debug.h"
#include "utils.h"
#include "block.h"
#include "model.h"
#include "thread.h"
#include "glutils.h"
#include "chunks.h"

// Normalized, not rotated
static ivec3 block_normals[] = {
        [CUBE_FRONT]    = {  0,  0,  1 },
        [CUBE_BACK]     = {  0,  0, -1 },
        [CUBE_TOP]      = {  0,  1,  0 },
        [CUBE_BOTTOM]   = {  0, -1,  0 },
        [CUBE_LEFT]     = { -1,  0,  0 },
        [CUBE_RIGHT]    = {  1,  0,  0 },
};

static inline float __local_to_gl(int x, int l)
{
        return l * (x + 1.0f / 2.0f);
}

/**
 * point_local_to_gl() - convert block/chunk local space to gl space
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
 * @param local: local point
 * @param edge_len: cube edge length
 * @param gl: output gl point
 */
void point_local_to_gl(const ivec3 local, int edge_len, vec3 gl)
{
        gl[X] = __local_to_gl(local[X], edge_len);
        gl[Y] = __local_to_gl(local[Y], edge_len);
        gl[Z] = __local_to_gl(local[Z], edge_len);
}

static inline float __gl_to_local(float y, int l)
{
        return y / (float)l - (1.0f / 2.0f);
}

void point_gl_to_local(const vec3 gl, int edge_len, vec3 local)
{
        local[X] = __gl_to_local(gl[X], edge_len);
        local[Y] = __gl_to_local(gl[Y], edge_len);
        local[Z] = __gl_to_local(gl[Z], edge_len);
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
        vec3 origin_gl = { 0.0f };

        if (!b || !blk_attr)
                return -EINVAL;

        memzero(b, sizeof(block));

        b->blk_attr = blk_attr;

        memcpy(b->origin_l, origin_block, sizeof(ivec3));
        point_local_to_gl(b->origin_l, BLOCK_EDGE_LEN_GLUNIT, origin_gl);

        /* TODO: axis */

        block_model_init(&b->model, origin_gl);

        return 0;
}

int block_deinit(block *b)
{
        if (!b)
                return -EINVAL;

        block_model_deinit(&b->model);

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

        c->state = CHUNK_INITED;

        pthread_rwlock_init(&c->rwlock, NULL);
        pthread_rwlock_init(&c->rwlock_gl, NULL);

        return 0;
}

int chunk_deinit(chunk *c)
{
        linklist_node *pos;

        if (!c)
                return -EINVAL;

        pthread_rwlock_wrlock(&c->rwlock);
        pthread_rwlock_wrlock(&c->rwlock_gl);

        linklist_for_each_node(pos, c->blocks->head) {
                block_deinit(pos->data);
        }

        gl_attr_buffer_delete(&c->glattr);

        if (!gl_vbo_is_empty(&c->glvbo)) {
                gl_vbo_deinit(&c->glvbo);
        }

        linklist_deinit(c->blocks);
        linklist_free(&c->blocks);

        seqlist_deinit(c->vertices);
        seqlist_free(&c->vertices);

        c->state = CHUNK_DEINITED;

        pthread_rwlock_unlock(&c->rwlock_gl);
        pthread_rwlock_unlock(&c->rwlock);

        pthread_rwlock_destroy(&c->rwlock);
        pthread_rwlock_destroy(&c->rwlock_gl);

        memzero(c, sizeof(chunk));

        return 0;
}

enum {
        L_WAIT = 0,
        L_NOWAIT,
};

static inline int chunk_state_get(chunk *c, int flag)
{
        int state = CHUNK_UNKNOWN;

        if (!c)
                return state;

        if (flag == L_NOWAIT) {
                if (pthread_rwlock_tryrdlock(&c->rwlock))
                        return state;
        } else {
                pthread_rwlock_rdlock(&c->rwlock);
        }

        state = c->state;

        pthread_rwlock_unlock(&c->rwlock);

        return state;
}

static inline int chunk_state_ready(chunk *c, int lock)
{
        chunk_state ready[] = {
                CHUNK_INITED,
                CHUNK_FLUSHED,
                CHUNK_NEED_FLUSH,
        };

        int ret = 0;

        if (!c)
                return ret;

        if (lock)
             pthread_rwlock_rdlock(&c->rwlock);

        for (uint32_t i = 0; i < ARRAY_SIZE(ready); ++i) {
                if (c->state == ready[i]) {
                        ret = 1;
                        break;
                }
        }

        if (lock)
                pthread_rwlock_unlock(&c->rwlock);

        return ret;
}

static inline linklist_node *chunk_get_block_node(chunk *c, ivec3 origin_b)
{
        linklist_node *pos;
        linklist_node *ret = NULL;

        if (!c)
                return ret;

        linklist_for_each_node(pos, c->blocks->head) {
                block *b = pos->data;

                if (ivec3_equal(b->origin_l, origin_b)) {
                        ret = pos;
                        break;
                }
        }

        return ret;
}

block *chunk_get_block(chunk *c, ivec3 origin_block)
{
        linklist_node *node;
        block *ret = NULL;

        pthread_rwlock_rdlock(&c->rwlock);

        if (linklist_is_empty(c->blocks))
                goto out;

        node = chunk_get_block_node(c, origin_block);
        if (node)
                ret = node->data;

out:
        pthread_rwlock_unlock(&c->rwlock);

        return ret;
}

block *chunk_add_block(chunk *c, block *b)
{
        block *ret;

        if (!c)
                return NULL;

        pthread_rwlock_wrlock(&c->rwlock);

        ret = linklist_append(c->blocks, b);
        if (ret == NULL)
                pr_err_func("linklist_append() failed\n");
        else
                c->state = CHUNK_NEED_UPDATE;

        pthread_rwlock_unlock(&c->rwlock);

        return ret;
}

int chunk_del_block(chunk *c, ivec3 origin_block)
{
        linklist_node *node;

        if (!c)
                return -EINVAL;

        pthread_rwlock_wrlock(&c->rwlock);

        node = chunk_get_block_node(c, origin_block);
        if (node == NULL) {
                pr_err_func("block (%d, %d, %d) not found in chunk (%d, %d, %d)\n",
                            origin_block[X], origin_block[Y], origin_block[Z],
                            c->origin_l[X], c->origin_l[Y], c->origin_l[Z]);
                goto out;
        }

        block_deinit(node->data);
        linklist_delete(c->blocks, node);

        c->state = CHUNK_NEED_UPDATE;

out:
        pthread_rwlock_unlock(&c->rwlock);

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
        linklist_node *pos;
        chunk *ret = NULL;

        if (linklist_is_empty(w->chunks))
                goto out;

        linklist_for_each_node(pos, w->chunks->head) {
                chunk *c = pos->data;

                if (ivec3_equal(c->origin_l, origin_chunk)) {
                        ret = c;
                        break;
                }
        }

out:
        return ret;
}

block *world_get_block(world *w, ivec3 origin_block)
{
        ivec3 origin_chunk = { 0 };
        chunk *c;
        block *b;

        if (!w)
                return NULL;

        block_in_chunk(origin_block, w->chunk_length, origin_chunk);

        c = world_get_chunk(w, origin_chunk);
        if (!c)
                return NULL;

        b = chunk_get_block(c, origin_block);

        return b;
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

static inline void chunk_vertices_alloc(chunk *c)
{
        seqlist_alloc(&c->vertices);
        seqlist_init(c->vertices, sizeof(vertex_attr), 128);
}

static inline void chunk_vertices_free(chunk *c)
{
        seqlist_deinit(c->vertices);
        seqlist_free(&c->vertices);
}

int chunk_vertices_pack(chunk *c)
{
        linklist_node *pos;

        if (!c)
                return 0;

        linklist_for_each_node(pos, c->blocks->head) {
                block *b = pos->data;

                block_model_face_init(&b->model);
                block_model_face_generate(&b->model, b->blk_attr);

                for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                        block_face *f = &(b->model.faces[i]);

                        if (!f->visible)
                                continue;

                        for (int j = 0; j < VERTICES_TRIANGULATE_QUAD; ++j) {
                                seqlist_append(c->vertices, &(f->vertices[j]));
                        }
                }

                block_model_face_deinit(&b->model);
        }

        seqlist_shrink(c->vertices);

        return 0;
}

int chunk_update(chunk *c)
{
        if (!c)
                return -EINVAL;

        pthread_rwlock_wrlock(&c->rwlock);

        if (c->state != CHUNK_NEED_UPDATE &&
            c->state != CHUNK_SCHED_UPDATE)
                goto unlock;

        c->state = CHUNK_UPDATING;

        chunk_vertices_alloc(c);
        chunk_vertices_pack(c);

        gl_vbo_init(&c->glvbo);
        gl_vbo_index(&c->glvbo, c->vertices->data,
                     (uint32_t)c->vertices->count_utilized);

        chunk_vertices_free(c);

        c->state = CHUNK_NEED_FLUSH;

unlock:
        pthread_rwlock_unlock(&c->rwlock);

        return 0;
}

static inline void block_near_origin_get(block *b, int f, ivec3 origin_near)
{
        if (!b)
                return;

        // XXX: if BLOCK_EDGE_LEN_GLUNIT != 1, this will be incorrect
        origin_near[X] += b->origin_l[X] + block_normals[f][X];
        origin_near[Y] += b->origin_l[Y] + block_normals[f][Y];
        origin_near[Z] += b->origin_l[Z] + block_normals[f][Z];
}

int chunk_cull_blocks(chunk *c, world *w)
{
        linklist_node *pos;

        if (!c)
                return -EINVAL;

        pthread_rwlock_rdlock(&c->rwlock);

        linklist_for_each_node(pos, c->blocks->head) {
                block *b = pos->data;

                for (int i = 0; i < CUBE_QUAD_FACES; ++i) {
                        block_face *f = &(b->model.faces[i]);
                        ivec3 o_near = { 0 };
                        block *b_near;

                        block_near_origin_get(b, i, o_near);
                        b_near = world_get_block(w, o_near);

                        if (!b_near)
                                f->visible = 1;
                        else
                                f->visible = 0;
                }
        }

        pthread_rwlock_unlock(&c->rwlock);

        return 0;
}

void *chunk_update_worker(void *data)
{
        typedef struct {
                world *w;
                chunk *c;
        } thread_arg;

        world *w = ((thread_arg *)data)->w;
        chunk *c = ((thread_arg *)data)->c;

        // Thread arg is on heap, free it
        memfree(&data);

        pr_info_func("chunk (%d, %d, %d)\n",
                      c->origin_l[X], c->origin_l[Y], c->origin_l[Z]);

        chunk_cull_blocks(c, w);
        chunk_update(c);

        return NULL;
}

int world_update_chunks(world *w)
{
        typedef struct {
                world *w;
                chunk *c;
        } thread_arg;

        linklist_node *pos;

        if (!w)
                return -EINVAL;

        linklist_for_each_node(pos, w->chunks->head) {
                chunk *c = pos->data;

                if (pthread_rwlock_trywrlock(&c->rwlock))
                        continue;

                if (c->state != CHUNK_NEED_UPDATE) {
                        pthread_rwlock_unlock(&c->rwlock);
                        continue;
                }

                c->state = CHUNK_SCHED_UPDATE;
                pthread_rwlock_unlock(&c->rwlock);

                thread_arg *arg = memalloc(sizeof(thread_arg));

                arg->w = w;
                arg->c = c;

                pthread_create_detached(chunk_update_worker, arg);
        }

        return 0;
}

void chunk_gl_attr_free(chunk *c)
{
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

int chunk_gl_attr_buffer_create(chunk *c)
{
        int ret;

        ret = gl_vbo_buffer_create(&c->glvbo, &c->glattr);
        if (ret == GL_FALSE)
                pr_err_func("failed to generate chunk VBO\n");

        return ret;
}

int chunk_gl_data_generate(chunk *c)
{
        if (!c)
                return -EINVAL;

        chunk_gl_attr_generate(&c->glattr, block_attr_get(BLOCK_DUMMY));
        chunk_gl_attr_buffer_create(c);

        gl_vbo_deinit(&c->glvbo);

        return 0;
}

int chunk_flush(chunk *c)
{
        if (unlikely(!c))
                return -EINVAL;

        if (pthread_rwlock_trywrlock(&c->rwlock))
                goto out;

        if (c->state != CHUNK_NEED_FLUSH)
                goto unlock;

        c->state = CHUNK_FLUSHING;

        pr_info_func("chunk (%d, %d, %d)\n",
                     c->origin_l[X], c->origin_l[Y], c->origin_l[Z]);

        // Since we gonna call draw call in the same thread
        // There is no point to grab rwlock_gl
        chunk_gl_attr_free(c);
        chunk_gl_data_generate(c);

        c->state = CHUNK_FLUSHED;

unlock:
        pthread_rwlock_unlock(&c->rwlock);

out:
        return 0;
}

int chunk_draw(chunk *c, mat4 mat_transform)
{
        gl_attr *glattr;

        if (unlikely(!c))
                return -EINVAL;

        if (pthread_rwlock_tryrdlock(&c->rwlock_gl))
                goto out;

        glattr = &c->glattr;

        // Validate gl buffer is generated or not
        // GL attr is generated during chunk flushing
        if (!glattr->vertex_count)
                goto unlock;

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

unlock:
        pthread_rwlock_unlock(&c->rwlock_gl);

out:
        return 0;
}

/**
 * world_draw_chunk() - draw world by VBO indexed chunks
 *
 * @param w: pointer to world container
 * @param mat_transform: perspective transform matrix
 * @return 0 on success
 */
int world_draw_chunks(world *w, mat4 mat_transform)
{
        linklist_node *pos;

        if (!w)
                return -EINVAL;

        linklist_for_each_node(pos, w->chunks->head) {
                chunk *c = pos->data;

                chunk_flush(c);
                chunk_draw(c, mat_transform);
        }

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
                chunk *c = pos->data;

                while (1) {
                        if (chunk_state_ready(c, 1))
                                break;
                }

                chunk_deinit(c);
        }

        linklist_deinit(w->chunks);
        linklist_free(&w->chunks);

        return 0;
}
