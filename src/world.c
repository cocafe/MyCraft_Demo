#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <errno.h>

#include <GL/glew.h>

#include "debug.h"
#include "model.h"
#include "utils.h"
#include "world.h"

static world_hierarchy super_flat_default[] = {
        { WORLD_HEIGHT_MIN,  1, BLOCK_BEDROCK },
        { WORLD_HEIGHT_AUTO, 7, BLOCK_STONE },
        { WORLD_HEIGHT_AUTO, 5, BLOCK_DIRT },
        { WORLD_HEIGHT_AUTO, 1, BLOCK_GRASS },
};

static world_hierarchy super_flat_debug[] = {
        { WORLD_HEIGHT_MIN,  1, BLOCK_BEDROCK },
        { WORLD_HEIGHT_AUTO, 1, BLOCK_TNT },
        { WORLD_HEIGHT_AUTO, 1, BLOCK_STONE },
        { WORLD_HEIGHT_AUTO, 1, BLOCK_DIRT },
        { WORLD_HEIGHT_AUTO, 1, BLOCK_GRASS },
};

static world_hierarchy super_flat_grass[] = {
        { WORLD_HEIGHT_MIN,  1, BLOCK_BEDROCK },
        { WORLD_HEIGHT_AUTO, 5, BLOCK_DIRT },
        { WORLD_HEIGHT_AUTO, 1, BLOCK_GRASS },
};

static world_preset super_flat_presets[] = {
        [SUPER_FLAT_DEFAULT] = { super_flat_default, 4 },
        [SUPER_FLAT_DEBUG]   = { super_flat_debug,   5 },
        [SUPER_FLAT_GRASS]   = { super_flat_grass,   3 },
};

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

int block_draw(block *b, mat4 mat_transform, int vbo_enabled)
{
        if (!b)
                return -EINVAL;

        if (!b->blk_attr->visible)
                return 0;

        if (b->model == NULL) {
                vec3 origin_gl = { 0 };
                b->model = memalloc(sizeof(block_model));
                if (!b->model)
                        return -ENOMEM;

                coordinate_local_to_gl(b->origin_l, BLOCK_EDGE_LEN_GLUNIT, origin_gl);

                block_model_init(b->model, origin_gl);
                block_model_faces_alloc(b->model);
                block_model_faces_generate(b->model, b->blk_attr);

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

int world_block_draw(world *w, mat4 mat_transform /* TODO: Render distance */)
{
        linklist_node *pos;

        if (!w)
                return -EINVAL;

        linklist_for_each_node(pos, w->chunks->head) {
                chunk_draw_block(pos->data, mat_transform);
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

world_preset *super_flat_preset_get(super_flat_preset_idx idx)
{
        if (idx < 0 || idx >= NR_SUPER_FLAT_PRESETS)
                return NULL;

        return &super_flat_presets[idx];
}

/**
 * super_flat_generate() - fill a fixed size super flat world chunks
 *
 * limited world size during generation,
 * can not be extended by discovering world
 *
 * @param w: pointer to world
 * @param idx: preset index
 * @param width: world width
 * @param length: world length
 * @return 0 on success
 */
int super_flat_generate(world *w, super_flat_preset_idx idx, int width, int length)
{
        block *b;
        world_preset *preset = super_flat_preset_get(idx);
        int32_t last_height = WORLD_HEIGHT_AUTO;

        if (!w || !preset)
                return 0;

        b = memalloc(sizeof(block));
        if (!b)
                return -ENOMEM;

        for (int i = 0; i < preset->hierarchy_count; ++i) {
                block_attr *type = block_attr_get(preset->hierarchy[i].type);
                int32_t h = preset->hierarchy[i].height;

                if (h == WORLD_HEIGHT_AUTO) {
                        h = last_height + 1; // if (h = -1 && i = 0), then (-1 + 1 = 0)
                }

                for (int j = h; j < (preset->hierarchy[i].thickness + h); ++j) {
                        for (int k = 0; k < width; ++k) {
                                for (int l = 0; l < length; ++l) {
                                        ivec3 origin_l = {
                                                [X] = k,
                                                [Y] = j,
                                                [Z] = l,
                                        };

                                        block_init(b, type, origin_l);
                                        world_add_block(w, b);
                                        // block_deinit(b);
                                }
                        }

                        last_height = j;
                }
        }

        memfree((void **)&b);

        return 0;
}
