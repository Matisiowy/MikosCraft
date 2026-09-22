#ifndef MIKOSCRAFT_WORLD_H
#define MIKOSCRAFT_WORLD_H

#include <stdint.h>
#include "block.h"

/*
 * ============================================================
 * MIKOSCRAFT WORLD FORMAT
 * ============================================================
 *
 * World is divided horizontally into chunks.
 *
 * Initial PS2 layout:
 *
 *      8 x 32 x 8 blocks
 *
 * 2048 BlockIDs per chunk = 2048 bytes.
 */

#define CHUNK_X 8
#define CHUNK_Y 32
#define CHUNK_Z 8

/*
 * Temporary resident chunk grid.
 *
 * 3x3 chunks = 24x24 blocks around origin.
 *
 * Later ChunkManager will move this window with the player.
 */

#define WORLD_CHUNK_RADIUS 1
#define WORLD_CHUNKS_X 3
#define WORLD_CHUNKS_Z 3
#define WORLD_CHUNK_COUNT \
    (WORLD_CHUNKS_X * WORLD_CHUNKS_Z)

/*
 * Compatibility.
 *
 * Some existing game code uses WORLD_Y.
 */
#define WORLD_Y CHUNK_Y


typedef struct {

    /*
     * Local coordinates:
     *
     * x: 0..7
     * y: 0..31
     * z: 0..7
     */

    BlockID blocks[CHUNK_X][CHUNK_Y][CHUNK_Z];

    /*
     * Chunk coordinates in the world.
     */
    int chunk_x;
    int chunk_z;

    uint8_t generated;
    uint8_t mesh_dirty;

} Chunk;


typedef struct {

    uint32_t seed;

    /*
     * Temporary fixed resident set.
     *
     * ChunkManager v2 will turn this into a moving cache.
     */
    Chunk chunks[WORLD_CHUNK_COUNT];

} World;


/*
 * Generate initial chunk window.
 */
void world_generate(
    World *world,
    uint32_t seed
);


/*
 * Get/set using GLOBAL block coordinates.
 *
 * Negative X/Z are valid.
 */
BlockID world_get_block(
    const World *world,
    int x,
    int y,
    int z
);


void world_set_block(
    World *world,
    int x,
    int y,
    int z,
    BlockID block
);


/*
 * Chunk lookup.
 */
Chunk *world_get_chunk(
    World *world,
    int chunk_x,
    int chunk_z
);


const Chunk *world_get_chunk_const(
    const World *world,
    int chunk_x,
    int chunk_z
);


/*
 * Coordinate conversion.
 *
 * Example:
 *
 * world -1
 *   -> chunk -1
 *   -> local 7
 */
int world_to_chunk_coord(int value);
int world_to_local_coord(int value);


/*
 * Procedural generation.
 */
void chunk_generate(
    Chunk *chunk,
    uint32_t seed,
    int chunk_x,
    int chunk_z
);


/*
 * Temporary resident-world bounds helper.
 *
 * Y must be valid and X/Z must currently be loaded.
 */
int world_is_inside(
    const World *world,
    int x,
    int y,
    int z
);

#endif
