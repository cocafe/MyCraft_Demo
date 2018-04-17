#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <errno.h>

#include <GL/glew.h>

#include "debug.h"
#include "block.h"
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
                                        world_add_block(w, b, 0);
                                }
                        }

                        last_height = j;
                }
        }

        world_update_trigger(w);

        memfree((void **)&b);

        return 0;
}
