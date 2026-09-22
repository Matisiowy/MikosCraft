#ifndef MIKOSCRAFT_BLOCK_H
#define MIKOSCRAFT_BLOCK_H

#include <stdint.h>

/*
 * Block IDs are part of the world/save format.
 * Once MikosCraft worlds exist, DO NOT casually reorder these.
 */
typedef uint8_t BlockID;

enum {
    BLOCK_AIR = 0,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_BEDROCK,
    BLOCK_COBBLESTONE,
    BLOCK_SAND,
    BLOCK_GRAVEL,
    BLOCK_PLANKS_OAK,
    BLOCK_LOG_OAK,
    BLOCK_LEAVES_OAK,
    BLOCK_GLASS,

    BLOCK_COUNT
};

typedef enum {
    BLOCK_FACE_TOP = 0,
    BLOCK_FACE_BOTTOM,
    BLOCK_FACE_NORTH,
    BLOCK_FACE_SOUTH,
    BLOCK_FACE_EAST,
    BLOCK_FACE_WEST
} BlockFace;

typedef struct {
    const char *name;

    uint8_t solid;
    uint8_t transparent;
    uint8_t breakable;

    /*
     * Temporary hardness scale.
     * Later this will interact with tools/mining speed.
     */
    uint8_t hardness;

    uint8_t texture_top;
    uint8_t texture_bottom;
    uint8_t texture_side;
} BlockDefinition;

void block_registry_init(void);

const BlockDefinition *block_get_definition(BlockID id);

uint8_t block_get_texture(
    BlockID id,
    BlockFace face
);

int block_is_solid(BlockID id);
int block_is_transparent(BlockID id);
int block_is_breakable(BlockID id);

#endif
