#ifndef MYCRAFT_DEMO_CHUNKS_H
#define MYCRAFT_DEMO_CHUNKS_H

#include <pthread.h>

#include "block.h"
#include "model.h"
#include "utils.h"
#include "glutils.h"

#define WORLD_CLEAR_COLOR               R_G_B_A_2GLSL(160, 192, 214, 255)
#define WORLD_SKY_COLOR                 WORLD_CLEAR_COLOR

#define WORLD_FOG_COLOR                 WORLD_SKY_COLOR
#define WORLD_FOG_DISTANCE              (64.0f)

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
        CHUNK_DEINITED,         // In case
        CHUNK_UPDATING,         // Updating vertices (WR locked)
        CHUNK_FLUSHED,          // GL data is updated
        CHUNK_FLUSHING,         // Updating GL data
        CHUNK_NEED_FLUSH,       // GL data is ready
        CHUNK_NEED_UPDATE,      // Need to update, or world just inited
        CHUNK_SCHED_UPDATE,     // Sched to update
        NR_CHUNK_STATES,
} chunk_state;

typedef struct chunk {
        ivec3                   origin_l;

        gl_vbo                  glvbo;
        gl_attr                 glattr;
        pthread_rwlock_t        rwlock_gl;

        seqlist                 *vertices;
        linklist                *blocks;

        chunk_state             state;

        pthread_rwlock_t        rwlock;
} chunk;

typedef struct world {
        int32_t                 height_min;
        int32_t                 height_max;

        int32_t                 chunk_length;
        linklist                *chunks;

        color_rgba              sky_color;

        color_rgba              fog_color;
        float                   fog_distance;

        int                     update_pending;
        pthread_t               update_worker;
        pthread_cond_t          update_cond;
        pthread_mutex_t         update_mutex;
        pthread_spinlock_t      update_spin;
} world;

void point_local_to_gl(const ivec3 local, int edge_len, vec3 gl);
void point_gl_to_local(const vec3 gl, int edge_len, vec3 local);

int block_init(block *b, block_attr *blk_attr, ivec3 origin_block);
int block_deinit(block *b);

int block_in_chunk(ivec3 origin_block, int chunk_length, ivec3 origin_chunk);

int chunk_init(chunk *c, ivec3 origin_chunk);
int chunk_deinit(chunk *c);

block *chunk_get_block(chunk *c, ivec3 origin_block, int wait);
block *chunk_add_block(chunk *c, block *b);
int chunk_del_block(chunk *c, ivec3 origin_block);
int chunk_cull_blocks(chunk *c, world *w);

chunk *world_add_chunk(world *w, ivec3 origin_chunk);
chunk *world_get_chunk(world *w, ivec3 origin_chunk);

int world_add_block(world *w, block *b, int update);
int world_del_block(world *w, ivec3 origin_block);
block *world_get_block(world *w, ivec3 origin_block, int wait);

int world_update_chunks(world *w, int detach);
int world_draw_chunks(world *w, vec3 camera, mat4 trans);

int world_update_trigger(world *w);
int world_worker_create(world *w);

int world_init(world *w);
int world_deinit(world *w);

#endif //MYCRAFT_DEMO_CHUNKS_H
