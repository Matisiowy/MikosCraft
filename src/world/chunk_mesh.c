#include <string.h>

#include "chunk_mesh.h"
#include "block.h"


static int block_is_air(
    const World *world,
    int x,
    int y,
    int z
)
{
    return
        world_get_block(
            world,
            x,
            y,
            z
        ) == BLOCK_AIR;
}


static void add_face(
    ChunkMesh *mesh,
    int x,
    int y,
    int z,
    BlockID block,
    int face
)
{
    ChunkMeshFace *out;

    if (
        mesh->face_count >=
        CHUNK_MESH_MAX_FACES
    ) {
        return;
    }

    out =
        &mesh->faces[
            mesh->face_count++
        ];

    out->x = (int16_t)x;
    out->y = (int16_t)y;
    out->z = (int16_t)z;

    out->block =
        (uint8_t)block;

    out->face =
        (uint8_t)face;
}


void chunk_mesh_build(
    World *world,
    Chunk *chunk,
    ChunkMesh *mesh
)
{
    int lx;
    int y;
    int lz;

    if (
        !world ||
        !chunk ||
        !mesh
    ) {
        return;
    }

    mesh->face_count = 0;

    mesh->chunk_x =
        chunk->chunk_x;

    mesh->chunk_z =
        chunk->chunk_z;

    for (
        lx = 0;
        lx < CHUNK_X;
        ++lx
    ) {

        for (
            y = 0;
            y < CHUNK_Y;
            ++y
        ) {

            for (
                lz = 0;
                lz < CHUNK_Z;
                ++lz
            ) {

                BlockID block;

                int x;
                int z;

                block =
                    chunk->blocks
                        [lx]
                        [y]
                        [lz];

                if (
                    block ==
                    BLOCK_AIR
                ) {
                    continue;
                }

                x =
                    chunk->chunk_x *
                    CHUNK_X +
                    lx;

                z =
                    chunk->chunk_z *
                    CHUNK_Z +
                    lz;

                /*
                 * TOP
                 */
                if (
                    block_is_air(
                        world,
                        x,
                        y + 1,
                        z
                    )
                ) {
                    add_face(
                        mesh,
                        x,y,z,
                        block,
                        MESH_FACE_TOP
                    );
                }

                /*
                 * BOTTOM
                 *
                 * Normally invisible underground,
                 * but required at exposed terrain.
                 */
                if (
                    y > 0 &&
                    block_is_air(
                        world,
                        x,
                        y - 1,
                        z
                    )
                ) {
                    add_face(
                        mesh,
                        x,y,z,
                        block,
                        MESH_FACE_BOTTOM
                    );
                }

                /*
                 * NORTH -Z
                 */
                if (
                    block_is_air(
                        world,
                        x,
                        y,
                        z - 1
                    )
                ) {
                    add_face(
                        mesh,
                        x,y,z,
                        block,
                        MESH_FACE_NORTH
                    );
                }

                /*
                 * SOUTH +Z
                 */
                if (
                    block_is_air(
                        world,
                        x,
                        y,
                        z + 1
                    )
                ) {
                    add_face(
                        mesh,
                        x,y,z,
                        block,
                        MESH_FACE_SOUTH
                    );
                }

                /*
                 * EAST +X
                 */
                if (
                    block_is_air(
                        world,
                        x + 1,
                        y,
                        z
                    )
                ) {
                    add_face(
                        mesh,
                        x,y,z,
                        block,
                        MESH_FACE_EAST
                    );
                }

                /*
                 * WEST -X
                 */
                if (
                    block_is_air(
                        world,
                        x - 1,
                        y,
                        z
                    )
                ) {
                    add_face(
                        mesh,
                        x,y,z,
                        block,
                        MESH_FACE_WEST
                    );
                }
            }
        }
    }

    mesh->valid = 1;

    chunk->mesh_dirty = 0;
}
