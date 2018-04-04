#ifndef MYCRAFT_DEMO_CHUNK_H
#define MYCRAFT_DEMO_CHUNK_H

#include "block.h"
#include "utils.h"
#include "model.h"

#define WORLD_HEIGHT_MAX                (BLOCK_EDGE_LEN_GLUNIT * 256)
#define WORLD_HEIGHT_MIN                (0)
#define WORLD_HEIGHT_AUTO               (-1)

#define WORLD_TOP                       (WORLD_HEIGHT_MAX)
#define WORLD_BOTTOM                    (WORLD_HEIGHT_MIN)

#define CHUNK_EDGE_LEN_GLUNIT           (BLOCK_EDGE_LEN_GLUNIT * 16)

typedef struct world_hierarchy {
        int32_t         height;
        int32_t         thickness;
        block_attr_idx      type;
} world_hierarchy;

typedef struct world_preset {
        world_hierarchy         *hierarchy;
        int32_t                 hierarchy_count;
} world_preset;

typedef enum super_flat_preset_idx {
        SUPER_FLAT_DEFAULT = 0,
        SUPER_FLAT_DEBUG,
        SUPER_FLAT_GRASS,
        NR_SUPER_FLAT_PRESETS,
} super_flat_preset_idx;

typedef struct block {
        ivec3           origin_l;
        vec3            axis[3];

        block_attr      *blk_attr;
        block_model     *model;
} block;

typedef enum chunk_state {
        CHUNK_UNKNOWN = 0,
        CHUNK_INITED,
        CHUNK_UPDATED,
        CHUNK_UPDATING,
        CHUNK_NEED_UPDATE,
        NR_CHUNK_STATES,
} chunk_state;

typedef struct chunk {
        ivec3           origin_l;

        // TODO: Need to be thread-safe
        chunk_state     state;

        gl_attr         glattr;

        seqlist         *vertices;
        linklist        *blocks;
} chunk;

typedef struct world {
        int32_t         height_min;
        int32_t         height_max;

        int32_t         chunk_length;
        linklist        *chunks;
} world;

void coordinate_local_to_gl(const ivec3 local, int edge_len, vec3 gl);

int block_init(block *b, block_attr *blk_attr, ivec3 origin_block);
int block_deinit(block *b);

int block_in_chunk(ivec3 origin_block, int chunk_length, ivec3 origin_chunk);

int block_draw(block *b, mat4 mat_transform, int vbo_enabled);
int block_dump(block *b);

int chunk_init(chunk *c, ivec3 origin_chunk);
int chunk_deinit(chunk *c);

linklist_node *chunk_get_block_node(chunk *c, ivec3 origin_block);

block *chunk_get_block(chunk *c, ivec3 origin_block);
block *chunk_add_block(chunk *c, block *b);
int chunk_del_block(chunk *c, ivec3 origin_block);
int chunk_draw_block(chunk *c, mat4 mat_transform);

int chunk_dump(chunk *c);

int world_init(world *w);
int world_deinit(world *w);

chunk *world_add_chunk(world *w, ivec3 origin_chunk);
chunk *world_get_chunk(world *w, ivec3 origin_chunk);

int world_add_block(world *w, block *b);
int world_del_block(world *w, ivec3 origin_block);

int world_draw_block(world *w, mat4 mat_transform);
int world_draw_chunk(world *w, mat4 mat_transform);

int world_update_chunk(world *w);

int world_block_dump(world *w);

world_preset *super_flat_preset_get(super_flat_preset_idx idx);
int super_flat_generate(world *w, super_flat_preset_idx idx, int width, int length);

#endif //MYCRAFT_DEMO_CHUNK_H
