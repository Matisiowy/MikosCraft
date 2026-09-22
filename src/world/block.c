#include <string.h>
#include "block.h"

static BlockDefinition g_blocks[BLOCK_COUNT];

static void block_define(
    BlockID id,
    const char *name,
    int solid,
    int transparent,
    int breakable,
    int hardness,
    int texture_top,
    int texture_bottom,
    int texture_side
)
{
    BlockDefinition *b;

    if (id >= BLOCK_COUNT)
        return;

    b = &g_blocks[id];

    b->name = name;
    b->solid = solid ? 1 : 0;
    b->transparent = transparent ? 1 : 0;
    b->breakable = breakable ? 1 : 0;
    b->hardness = (uint8_t)hardness;

    b->texture_top = (uint8_t)texture_top;
    b->texture_bottom = (uint8_t)texture_bottom;
    b->texture_side = (uint8_t)texture_side;
}

void block_registry_init(void)
{
    memset(g_blocks, 0, sizeof(g_blocks));

    /*
     * Atlas mapping:
     *
     *  0 grass_top
     *  1 grass_side
     *  2 dirt
     *  3 stone
     *  4 bedrock
     *  5 cobblestone
     *  6 sand
     *  7 gravel
     *  8 planks_oak
     *  9 log_oak
     * 10 log_oak_top
     * 11 leaves_oak
     * 12 glass
     */

    block_define(
        BLOCK_AIR,
        "Air",
        0, 1, 0, 0,
        0, 0, 0
    );

    block_define(
        BLOCK_GRASS,
        "Grass Block",
        1, 0, 1, 2,
        0, 2, 1
    );

    block_define(
        BLOCK_DIRT,
        "Dirt",
        1, 0, 1, 2,
        2, 2, 2
    );

    block_define(
        BLOCK_STONE,
        "Stone",
        1, 0, 1, 4,
        3, 3, 3
    );

    block_define(
        BLOCK_BEDROCK,
        "Bedrock",
        1, 0, 0, 255,
        4, 4, 4
    );

    block_define(
        BLOCK_COBBLESTONE,
        "Cobblestone",
        1, 0, 1, 4,
        5, 5, 5
    );

    block_define(
        BLOCK_SAND,
        "Sand",
        1, 0, 1, 2,
        6, 6, 6
    );

    block_define(
        BLOCK_GRAVEL,
        "Gravel",
        1, 0, 1, 2,
        7, 7, 7
    );

    block_define(
        BLOCK_PLANKS_OAK,
        "Oak Planks",
        1, 0, 1, 3,
        8, 8, 8
    );

    block_define(
        BLOCK_LOG_OAK,
        "Oak Log",
        1, 0, 1, 3,
        10, 10, 9
    );

    block_define(
        BLOCK_LEAVES_OAK,
        "Oak Leaves",
        1, 1, 1, 1,
        11, 11, 11
    );

    block_define(
        BLOCK_GLASS,
        "Glass",
        1, 1, 1, 1,
        12, 12, 12
    );
}

const BlockDefinition *block_get_definition(BlockID id)
{
    if (id >= BLOCK_COUNT)
        return &g_blocks[BLOCK_AIR];

    return &g_blocks[id];
}

uint8_t block_get_texture(
    BlockID id,
    BlockFace face
)
{
    const BlockDefinition *b;

    b = block_get_definition(id);

    if (face == BLOCK_FACE_TOP)
        return b->texture_top;

    if (face == BLOCK_FACE_BOTTOM)
        return b->texture_bottom;

    return b->texture_side;
}

int block_is_solid(BlockID id)
{
    return block_get_definition(id)->solid != 0;
}

int block_is_transparent(BlockID id)
{
    return block_get_definition(id)->transparent != 0;
}

int block_is_breakable(BlockID id)
{
    return block_get_definition(id)->breakable != 0;
}
