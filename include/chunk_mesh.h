#ifndef MIKOSCRAFT_CHUNK_MESH_H
#define MIKOSCRAFT_CHUNK_MESH_H

#include <stdint.h>
#include "world.h"

/*
 * ============================================================
 * CHUNK MESH
 * ============================================================
 *
 * Chunk is 8 x 32 x 8.
 *
 * Worst theoretical visible faces:
 *
 * 2048 blocks * 6 faces = 12288.
 *
 * Real terrain is dramatically smaller.
 *
 * We deliberately store FACES instead of final projected
 * vertices because camera transformation changes every frame.
 *
 * Expensive block-neighbour discovery however does NOT.
 */

#define CHUNK_MESH_MAX_FACES 12288

typedef enum {
    MESH_FACE_TOP = 0,
    MESH_FACE_BOTTOM,
    MESH_FACE_NORTH,
    MESH_FACE_SOUTH,
    MESH_FACE_EAST,
    MESH_FACE_WEST
} ChunkMeshFaceDirection;


typedef struct {

    /*
     * GLOBAL block coordinates.
     */
    int16_t x;
    int16_t y;
    int16_t z;

    uint8_t block;
    uint8_t face;

} ChunkMeshFace;


typedef struct {

    ChunkMeshFace faces[
        CHUNK_MESH_MAX_FACES
    ];

    int face_count;

    int chunk_x;
    int chunk_z;

    uint8_t valid;

} ChunkMesh;


void chunk_mesh_build(
    World *world,
    Chunk *chunk,
    ChunkMesh *mesh
);

#endif
