#ifndef MYCRAFT_DEMO_CHUNKS_H
#define MYCRAFT_DEMO_CHUNKS_H

#include "block.h"
#include "model.h"
#include "utils.h"

#define WORLD_HEIGHT_MAX                (BLOCK_EDGE_LEN_GLUNIT * 256)
#define WORLD_HEIGHT_MIN                (0)
#define WORLD_HEIGHT_AUTO               (-1)

#define WORLD_TOP                       (WORLD_HEIGHT_MAX)
#define WORLD_BOTTOM                    (WORLD_HEIGHT_MIN)

#define CHUNK_EDGE_LEN_GLUNIT           (BLOCK_EDGE_LEN_GLUNIT * 16)

typedef struct block {
        ivec3           origin_l;
        vec3            axis[3];

        block_attr      *blk_attr;
        block_model     model;
} block;

typedef enum chunk_state {
        CHUNK_UNKNOWN = 0,      // Undefined
        CHUNK_INITED,           // Ready
        CHUNK_PREPARED,         // Ready
        CHUNK_PREPARING,        // Updating vertices (WR locked)
        CHUNK_UPDATED,          // Ready
        CHUNK_UPDATING,         // Updating GL data
        CHUNK_NEED_UPDATE,      // Need to update, or world just inited
        NR_CHUNK_STATES,
} chunk_state;

typedef struct chunk {
        ivec3                   origin_l;

        gl_vbo                  glvbo;
        gl_attr                 glattr;

        seqlist                 *vertices;
        linklist                *blocks;

        chunk_state             state;
        pthread_rwlock_t        rwlock;
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

int chunk_init(chunk *c, ivec3 origin_chunk);
int chunk_deinit(chunk *c);

block *chunk_get_block(chunk *c, ivec3 origin_block);
block *chunk_add_block(chunk *c, block *b);
int chunk_del_block(chunk *c, ivec3 origin_block);

chunk *world_add_chunk(world *w, ivec3 origin_chunk);
chunk *world_get_chunk(world *w, ivec3 origin_chunk);

int world_add_block(world *w, block *b);
int world_del_block(world *w, ivec3 origin_block);

int chunk_set_blocks(chunk *c, world *w);
int world_set_blocks(world *w, int init, int trap);

int world_prepare_chunks(world *w);
int world_update_chunks(world *w);
int world_draw_chunks(world *w, mat4 mat_transform);

int world_init(world *w);
int world_deinit(world *w);

#endif //MYCRAFT_DEMO_CHUNKS_H
