#include <stdint.h>
#include <string.h>

#include "world.h"


/*
 * ============================================================
 * COORDINATE MATH
 * ============================================================
 *
 * C integer division truncates toward zero.
 *
 * We need mathematical floor division.
 *
 * Examples for chunk size 8:
 *
 *   8 ->  1
 *   7 ->  0
 *   0 ->  0
 *  -1 -> -1
 *  -8 -> -1
 *  -9 -> -2
 */

static int floor_div(
    int value,
    int divisor
)
{
    int q;
    int r;

    q = value / divisor;
    r = value % divisor;

    if (
        r != 0 &&
        ((r < 0) != (divisor < 0))
    ) {
        q--;
    }

    return q;
}


static int floor_mod(
    int value,
    int divisor
)
{
    int r;

    r = value % divisor;

    if (r < 0)
        r += divisor;

    return r;
}


int world_to_chunk_coord(int value)
{
    return floor_div(value, CHUNK_X);
}


int world_to_local_coord(int value)
{
    return floor_mod(value, CHUNK_X);
}


/*
 * ============================================================
 * HASH / NOISE HELPERS
 * ============================================================
 */

static uint32_t hash32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;

    x ^= x >> 15;
    x *= 0x846ca68bU;

    x ^= x >> 16;

    return x;
}


static uint32_t terrain_hash(
    uint32_t seed,
    int x,
    int z
)
{
    uint32_t h;

    h = seed;

    h ^= (uint32_t)x * 0x9E3779B1U;
    h ^= (uint32_t)z * 0x85EBCA77U;

    return hash32(h);
}


/*
 * Integer value noise.
 *
 * Returns roughly 0..255.
 *
 * This is deliberately cheap for PS2.
 */

static int noise_point(
    uint32_t seed,
    int x,
    int z
)
{
    return
        (int)(
            terrain_hash(seed, x, z)
            & 255U
        );
}


/*
 * Linear interpolation using integer fixed point.
 *
 * t: 0..256
 */
static int lerp256(
    int a,
    int b,
    int t
)
{
    return
        a +
        (((b - a) * t) >> 8);
}


/*
 * Smooth-ish 2D value noise.
 *
 * scale controls feature size.
 */
static int value_noise(
    uint32_t seed,
    int x,
    int z,
    int scale
)
{
    int gx;
    int gz;

    int lx;
    int lz;

    int a;
    int b;
    int c;
    int d;

    int ab;
    int cd;

    int tx;
    int tz;

    gx = floor_div(x, scale);
    gz = floor_div(z, scale);

    lx = floor_mod(x, scale);
    lz = floor_mod(z, scale);

    tx = (lx * 256) / scale;
    tz = (lz * 256) / scale;

    a = noise_point(seed, gx,     gz);
    b = noise_point(seed, gx + 1, gz);
    c = noise_point(seed, gx,     gz + 1);
    d = noise_point(seed, gx + 1, gz + 1);

    ab = lerp256(a, b, tx);
    cd = lerp256(c, d, tx);

    return lerp256(ab, cd, tz);
}


/*
 * ============================================================
 * TERRAIN
 * ============================================================
 */

static int terrain_height(
    uint32_t seed,
    int world_x,
    int world_z
)
{
    int broad;
    int medium;
    int detail;

    int height;

    /*
     * Three cheap octaves.
     *
     * IMPORTANT:
     * coordinates are GLOBAL world coordinates.
     *
     * Therefore chunk borders match perfectly.
     */

    broad =
        value_noise(
            seed ^ 0xA341316CU,
            world_x,
            world_z,
            32
        );

    medium =
        value_noise(
            seed ^ 0xC8013EA4U,
            world_x,
            world_z,
            16
        );

    detail =
        value_noise(
            seed ^ 0xAD90777DU,
            world_x,
            world_z,
            8
        );

    /*
     * Base around Y=9.
     *
     * broad  : roughly +/- 3
     * medium : roughly +/- 2
     * detail : roughly +/- 1
     */

    height = 9;

    height += (broad - 128) / 42;
    height += (medium - 128) / 64;
    height += (detail - 128) / 128;

    if (height < 4)
        height = 4;

    if (height > CHUNK_Y - 3)
        height = CHUNK_Y - 3;

    return height;
}


/*
 * ============================================================
 * CHUNK GENERATION
 * ============================================================
 */

void chunk_generate(
    Chunk *chunk,
    uint32_t seed,
    int chunk_x,
    int chunk_z
)
{
    int lx;
    int y;
    int lz;

    if (!chunk)
        return;

    memset(
        chunk->blocks,
        0,
        sizeof(chunk->blocks)
    );

    chunk->chunk_x = chunk_x;
    chunk->chunk_z = chunk_z;

    for (
        lx = 0;
        lx < CHUNK_X;
        ++lx
    ) {

        for (
            lz = 0;
            lz < CHUNK_Z;
            ++lz
        ) {

            int world_x;
            int world_z;

            int height;

            world_x =
                chunk_x * CHUNK_X +
                lx;

            world_z =
                chunk_z * CHUNK_Z +
                lz;

            height =
                terrain_height(
                    seed,
                    world_x,
                    world_z
                );

            for (
                y = 0;
                y < CHUNK_Y;
                ++y
            ) {

                BlockID block;

                block = BLOCK_AIR;

                if (y == 0) {

                    block =
                        BLOCK_BEDROCK;

                }

                else if (
                    y <
                    height - 3
                ) {

                    block =
                        BLOCK_STONE;

                }

                else if (
                    y < height
                ) {

                    block =
                        BLOCK_DIRT;

                }

                else if (
                    y == height
                ) {

                    block =
                        BLOCK_GRASS;

                }

                chunk->blocks[lx][y][lz] =
                    block;
            }
        }
    }

    chunk->generated = 1;
    chunk->mesh_dirty = 1;
}


/*
 * ============================================================
 * CHUNK LOOKUP
 * ============================================================
 */

Chunk *world_get_chunk(
    World *world,
    int chunk_x,
    int chunk_z
)
{
    int i;

    if (!world)
        return 0;

    for (
        i = 0;
        i < WORLD_CHUNK_COUNT;
        ++i
    ) {

        Chunk *chunk;

        chunk =
            &world->chunks[i];

        if (
            chunk->generated &&
            chunk->chunk_x == chunk_x &&
            chunk->chunk_z == chunk_z
        ) {

            return chunk;
        }
    }

    return 0;
}


const Chunk *world_get_chunk_const(
    const World *world,
    int chunk_x,
    int chunk_z
)
{
    int i;

    if (!world)
        return 0;

    for (
        i = 0;
        i < WORLD_CHUNK_COUNT;
        ++i
    ) {

        const Chunk *chunk;

        chunk =
            &world->chunks[i];

        if (
            chunk->generated &&
            chunk->chunk_x == chunk_x &&
            chunk->chunk_z == chunk_z
        ) {

            return chunk;
        }
    }

    return 0;
}


/*
 * ============================================================
 * WORLD BLOCK ACCESS
 * ============================================================
 */

BlockID world_get_block(
    const World *world,
    int x,
    int y,
    int z
)
{
    int chunk_x;
    int chunk_z;

    int local_x;
    int local_z;

    const Chunk *chunk;

    if (!world)
        return BLOCK_AIR;

    if (
        y < 0 ||
        y >= CHUNK_Y
    ) {
        return BLOCK_AIR;
    }

    chunk_x =
        world_to_chunk_coord(x);

    chunk_z =
        world_to_chunk_coord(z);

    local_x =
        world_to_local_coord(x);

    local_z =
        world_to_local_coord(z);

    chunk =
        world_get_chunk_const(
            world,
            chunk_x,
            chunk_z
        );

    if (!chunk)
        return BLOCK_AIR;

    return
        chunk->blocks
            [local_x]
            [y]
            [local_z];
}


void world_set_block(
    World *world,
    int x,
    int y,
    int z,
    BlockID block
)
{
    int chunk_x;
    int chunk_z;

    int local_x;
    int local_z;

    Chunk *chunk;

    if (!world)
        return;

    if (
        y < 0 ||
        y >= CHUNK_Y
    ) {
        return;
    }

    if (block >= BLOCK_COUNT)
        block = BLOCK_AIR;

    chunk_x =
        world_to_chunk_coord(x);

    chunk_z =
        world_to_chunk_coord(z);

    local_x =
        world_to_local_coord(x);

    local_z =
        world_to_local_coord(z);

    chunk =
        world_get_chunk(
            world,
            chunk_x,
            chunk_z
        );

    if (!chunk)
        return;

    chunk->blocks
        [local_x]
        [y]
        [local_z] =
            block;

    chunk->mesh_dirty = 1;

    /*
     * Zmiana bloku na krawędzi chunka wpływa również
     * na widoczność ścian w sąsiednim chunku.
     */

    if (local_x == 0) {

        Chunk *neighbor =
            world_get_chunk(
                world,
                chunk_x - 1,
                chunk_z
            );

        if (neighbor)
            neighbor->mesh_dirty = 1;
    }

    if (local_x == CHUNK_X - 1) {

        Chunk *neighbor =
            world_get_chunk(
                world,
                chunk_x + 1,
                chunk_z
            );

        if (neighbor)
            neighbor->mesh_dirty = 1;
    }

    if (local_z == 0) {

        Chunk *neighbor =
            world_get_chunk(
                world,
                chunk_x,
                chunk_z - 1
            );

        if (neighbor)
            neighbor->mesh_dirty = 1;
    }

    if (local_z == CHUNK_Z - 1) {

        Chunk *neighbor =
            world_get_chunk(
                world,
                chunk_x,
                chunk_z + 1
            );

        if (neighbor)
            neighbor->mesh_dirty = 1;
    }
}


/*
 * Is the global block currently resident?
 */
int world_is_inside(
    const World *world,
    int x,
    int y,
    int z
)
{
    int cx;
    int cz;

    if (!world)
        return 0;

    if (
        y < 0 ||
        y >= CHUNK_Y
    ) {
        return 0;
    }

    cx =
        world_to_chunk_coord(x);

    cz =
        world_to_chunk_coord(z);

    return
        world_get_chunk_const(
            world,
            cx,
            cz
        ) != 0;
}


/*
 * ============================================================
 * INITIAL WORLD
 * ============================================================
 */

void world_generate(
    World *world,
    uint32_t seed
)
{
    int cx;
    int cz;

    int index;

    if (!world)
        return;

    memset(
        world,
        0,
        sizeof(*world)
    );

    world->seed = seed;

    index = 0;

    /*
     * Initial resident grid:
     *
     * (-1,-1) (0,-1) (1,-1)
     * (-1, 0) (0, 0) (1, 0)
     * (-1, 1) (0, 1) (1, 1)
     */

    for (
        cz = -WORLD_CHUNK_RADIUS;
        cz <= WORLD_CHUNK_RADIUS;
        ++cz
    ) {

        for (
            cx = -WORLD_CHUNK_RADIUS;
            cx <= WORLD_CHUNK_RADIUS;
            ++cx
        ) {

            chunk_generate(
                &world->chunks[index],
                seed,
                cx,
                cz
            );

            index++;
        }
    }
}
