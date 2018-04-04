#ifndef MYCRAFT_DEMO_CHUNK_H
#define MYCRAFT_DEMO_CHUNK_H

#include <pthread.h>

#include "block.h"
#include "chunks.h"
#include "utils.h"

typedef struct world_hierarchy {
        int32_t         height;
        int32_t         thickness;
        block_attr_idx  type;
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

world_preset *super_flat_preset_get(super_flat_preset_idx idx);
int super_flat_generate(world *w, super_flat_preset_idx idx, int width, int length);

#endif //MYCRAFT_DEMO_CHUNK_H
