#include <math.h>
#include <gsKit.h>
#include <gsInline.h>

#include "renderer.h"
#include "world.h"
#include "chunk_mesh.h"
#include "block.h"
#include "voxel_renderer.h"
#include "mikos_texture.h"

typedef struct {
    float x;
    float y;
    float z;
} Vec3;


/*
 * ============================================================
 * CAMERA FRAME CACHE
 * ============================================================
 *
 * yaw/pitch nie zmieniają się podczas renderowania jednej
 * klatki. Nie liczymy więc sin/cos dla każdego vertexa.
 */

typedef struct {
    float cy;
    float sy;
    float cp;
    float sp;
} CameraFrameCache;

static CameraFrameCache g_camera_cache;


static void camera_cache_update(
    const Camera *camera
)
{
    g_camera_cache.cy = cosf(camera->yaw);
    g_camera_cache.sy = sinf(camera->yaw);

    g_camera_cache.cp = cosf(camera->pitch);
    g_camera_cache.sp = sinf(camera->pitch);
}

typedef struct {
    float x;
    float y;
    float z;

    float u;
    float v;
} ClipVertex;


typedef struct {
    float x;
    float y;

    int z;

    /*
     * Perspective correction value for PS2 GS.
     * q = 1 / camera-space depth.
     */
    float q;

    int visible;
} ScreenVertex;


enum {
    FACE_TOP = 0,
    FACE_BOTTOM,
    FACE_NORTH,
    FACE_SOUTH,
    FACE_EAST,
    FACE_WEST
};


static int block_texture(BlockID block, int face)
{
    BlockFace block_face;

    switch (face) {
        case FACE_TOP:
            block_face = BLOCK_FACE_TOP;
            break;

        case FACE_BOTTOM:
            block_face = BLOCK_FACE_BOTTOM;
            break;

        case FACE_NORTH:
            block_face = BLOCK_FACE_NORTH;
            break;

        case FACE_SOUTH:
            block_face = BLOCK_FACE_SOUTH;
            break;

        case FACE_EAST:
            block_face = BLOCK_FACE_EAST;
            break;

        case FACE_WEST:
        default:
            block_face = BLOCK_FACE_WEST;
            break;
    }

    return (int)block_get_texture(
        block,
        block_face
    );
}


/*
 * ============================================================
 * WORLD -> CAMERA SPACE
 * ============================================================
 */

static ClipVertex world_to_camera(
    Vec3 v,
    Camera *camera,
    float u,
    float tex_v
)
{
    ClipVertex out;

    float dx;
    float dy;
    float dz;

    float rx;
    float rz;


    dx = v.x - camera->x;
    dy = v.y - camera->y;
    dz = v.z - camera->z;


    /*
     * Yaw
     */
    rx =
        dx * g_camera_cache.cy -
        dz * g_camera_cache.sy;

    rz =
        dx * g_camera_cache.sy +
        dz * g_camera_cache.cy;


    /*
     * Pitch
     */
    out.x = rx;

    out.y =
        dy * g_camera_cache.cp -
        rz * g_camera_cache.sp;

    out.z =
        dy * g_camera_cache.sp +
        rz * g_camera_cache.cp;


    out.u = u;
    out.v = tex_v;

    return out;
}


/*
 * ============================================================
 * CAMERA SPACE -> SCREEN
 * ============================================================
 */

static ScreenVertex project_camera_vertex(
    ClipVertex v
)
{
    ScreenVertex out;

    float focal;
    float depth;


    if (v.z <= 0.0f) {

        out.x = 0.0f;
        out.y = 0.0f;
        out.z = 0;
        out.q = 0.0f;
        out.visible = 0;

        return out;
    }


    focal = 390.0f;


    out.x =
        320.0f +
        (v.x / v.z) * focal;

    out.y =
        256.0f -
        (v.y / v.z) * focal;


    depth =
        65535.0f -
        v.z * 1024.0f;

    if (depth < 1.0f)
        depth = 1.0f;

    if (depth > 65535.0f)
        depth = 65535.0f;


    out.z = (int)depth;

    out.q = 1.0f / v.z;

    out.visible = 1;

    return out;
}


/*
 * Stara funkcja zostaje chwilowo dla kodu legacy draw_block().
 */
static ScreenVertex project_vertex(
    Vec3 v,
    Camera *camera
)
{
    ScreenVertex out;

    float dx;
    float dy;
    float dz;

    float cy;
    float sy;

    float cp;
    float sp;

    float rx;
    float ry;
    float rz;

    float ry2;
    float rz2;

    float focal;

    dx = v.x - camera->x;
    dy = v.y - camera->y;
    dz = v.z - camera->z;

    cy = g_camera_cache.cy;
    sy = g_camera_cache.sy;

    /*
     * YAW
     */
    rx =
        dx * cy -
        dz * sy;

    rz =
        dx * sy +
        dz * cy;

    /*
     * PITCH
     */
    cp = g_camera_cache.cp;
    sp = g_camera_cache.sp;

    ry2 =
        dy * cp -
        rz * sp;

    rz2 =
        dy * sp +
        rz * cp;

    /*
     * Near plane
     */
    if (rz2 <= 0.10f) {

        out.x = 0;
        out.y = 0;
        out.z = 0;
        out.q = 0.0f;
        out.visible = 0;

        return out;
    }

    focal = 390.0f;

    out.x =
        320.0f +
        (rx / rz2) * focal;

    out.y =
        256.0f -
        (ry2 / rz2) * focal;

    /*
     * PS2 GS depth.
     *
     * Near geometry receives larger Z.
     */
    {
        float depth =
            65535.0f -
            rz2 * 1024.0f;

        if (depth < 1.0f)
            depth = 1.0f;

        if (depth > 65535.0f)
            depth = 65535.0f;

        out.z = (int)depth;
    }

    /*
     * Q used by the GS for perspective-correct texture mapping.
     */
    out.q = 1.0f / rz2;

    out.visible = 1;

    return out;
}


static void atlas_uv(
    int tile,
    float *u0,
    float *v0,
    float *u1,
    float *v1
)
{
    int tx;
    int ty;

    tx = tile & 15;
    ty = tile >> 4;

    /*
     * Pół piksela do środka kafelka.
     * Zapobiega bleedingowi sąsiednich tiles.
     */

    *u0 = tx * 16.0f + 0.5f;
    *v0 = ty * 16.0f + 0.5f;

    *u1 = tx * 16.0f + 15.5f;
    *v1 = ty * 16.0f + 15.5f;
}


static void textured_triangle(
    ScreenVertex a,
    ScreenVertex b,
    ScreenVertex c,

    float au,
    float av,

    float bu,
    float bv,

    float cu,
    float cv,

    int brightness
)
{
    float cross;

    GSPRIMSTQPOINT vertices[3];

    float aq;
    float bq;
    float cq;

    if (
        !a.visible ||
        !b.visible ||
        !c.visible
    ) {
        return;
    }

    /*
     * Screen-space backface culling.
     */
    cross =
        (b.x - a.x) *
        (c.y - a.y) -
        (b.y - a.y) *
        (c.x - a.x);

    if (cross <= 0.0f)
        return;

    aq = a.q;
    bq = b.q;
    cq = c.q;

    /*
     * PS2 GS perspective-correct texture coordinates.
     *
     * GS interpolates S, T and Q and obtains the final
     * texture coordinate from S/Q and T/Q.
     *
     * Therefore:
     *
     *      Q = 1/Z
     *      S = normalized_U * Q
     *      T = normalized_V * Q
     *
     * Atlas is 256x256.
     */

    vertices[0].rgbaq =
        color_to_RGBAQ(
            brightness,
            brightness,
            brightness,
            0x80,
            aq
        );

    vertices[0].stq =
        vertex_to_STQ(
            (au / 256.0f) * aq,
            (av / 256.0f) * aq
        );

    vertices[0].xyz2 =
        vertex_to_XYZ2(
            g_gs,
            a.x,
            a.y,
            a.z
        );


    vertices[1].rgbaq =
        color_to_RGBAQ(
            brightness,
            brightness,
            brightness,
            0x80,
            bq
        );

    vertices[1].stq =
        vertex_to_STQ(
            (bu / 256.0f) * bq,
            (bv / 256.0f) * bq
        );

    vertices[1].xyz2 =
        vertex_to_XYZ2(
            g_gs,
            b.x,
            b.y,
            b.z
        );


    vertices[2].rgbaq =
        color_to_RGBAQ(
            brightness,
            brightness,
            brightness,
            0x80,
            cq
        );

    vertices[2].stq =
        vertex_to_STQ(
            (cu / 256.0f) * cq,
            (cv / 256.0f) * cq
        );

    vertices[2].xyz2 =
        vertex_to_XYZ2(
            g_gs,
            c.x,
            c.y,
            c.z
        );

    gsKit_prim_list_triangle_goraud_texture_stq_3d(
        g_gs,
        &g_terrain_texture,
        3,
        vertices
    );
}


/*
 * ============================================================
 * NEAR PLANE CLIPPING
 * ============================================================
 */

#define VOXEL_NEAR_PLANE 0.10f


static ClipVertex clip_intersection(
    ClipVertex a,
    ClipVertex b
)
{
    ClipVertex out;

    float t;


    t =
        (VOXEL_NEAR_PLANE - a.z) /
        (b.z - a.z);


    out.x =
        a.x +
        (b.x - a.x) * t;

    out.y =
        a.y +
        (b.y - a.y) * t;

    out.z =
        VOXEL_NEAR_PLANE;


    /*
     * UV również musi zostać przecięte w tym samym
     * miejscu geometrycznym.
     */
    out.u =
        a.u +
        (b.u - a.u) * t;

    out.v =
        a.v +
        (b.v - a.v) * t;


    return out;
}


/*
 * Sutherland-Hodgman dla jednej płaszczyzny:
 *
 *     Z >= NEAR
 *
 * Triangle może po clippingu mieć:
 *
 * 0 vertexów -> niewidoczny
 * 3 vertexy  -> triangle
 * 4 vertexy  -> quad -> 2 triangles
 */

static int clip_triangle_near(
    const ClipVertex input[3],
    ClipVertex output[4]
)
{
    int out_count;
    int i;

    ClipVertex previous;
    int previous_inside;


    out_count = 0;

    previous = input[2];

    previous_inside =
        previous.z >=
        VOXEL_NEAR_PLANE;


    for (
        i = 0;
        i < 3;
        ++i
    ) {

        ClipVertex current;
        int current_inside;


        current = input[i];

        current_inside =
            current.z >=
            VOXEL_NEAR_PLANE;


        if (current_inside) {

            if (!previous_inside) {

                output[out_count++] =
                    clip_intersection(
                        previous,
                        current
                    );
            }


            output[out_count++] =
                current;

        } else if (previous_inside) {

            output[out_count++] =
                clip_intersection(
                    previous,
                    current
                );
        }


        previous =
            current;

        previous_inside =
            current_inside;
    }


    return out_count;
}


/*
 * Render jednego triangle'a w WORLD SPACE,
 * ale z prawidłowym near clippingiem.
 */

static void textured_triangle_clipped(
    Camera *camera,

    Vec3 a,
    Vec3 b,
    Vec3 c,

    float au,
    float av,

    float bu,
    float bv,

    float cu,
    float cv,

    int brightness
)
{
    ClipVertex input[3];
    ClipVertex clipped[4];

    int count;
    int i;


    input[0] =
        world_to_camera(
            a,
            camera,
            au,
            av
        );

    input[1] =
        world_to_camera(
            b,
            camera,
            bu,
            bv
        );

    input[2] =
        world_to_camera(
            c,
            camera,
            cu,
            cv
        );


    count =
        clip_triangle_near(
            input,
            clipped
        );


    if (count < 3)
        return;


    /*
     * Triangle fan.
     *
     * 3 verts -> 1 triangle
     * 4 verts -> 2 triangles
     */

    for (
        i = 1;
        i < count - 1;
        ++i
    ) {

        ScreenVertex sa;
        ScreenVertex sb;
        ScreenVertex sc;


        sa =
            project_camera_vertex(
                clipped[0]
            );

        sb =
            project_camera_vertex(
                clipped[i]
            );

        sc =
            project_camera_vertex(
                clipped[i + 1]
            );


        textured_triangle(
            sa,
            sb,
            sc,

            clipped[0].u,
            clipped[0].v,

            clipped[i].u,
            clipped[i].v,

            clipped[i + 1].u,
            clipped[i + 1].v,

            brightness
        );
    }
}


static void textured_quad(
    Camera *camera,

    Vec3 a,
    Vec3 b,
    Vec3 c,
    Vec3 d,

    int tile,
    int brightness
)
{
    float u0;
    float v0;
    float u1;
    float v1;


    atlas_uv(
        tile,
        &u0,
        &v0,
        &u1,
        &v1
    );


    /*
     * Quad = dwa triangles.
     *
     * Każdy triangle jest najpierw transformowany
     * do camera-space, potem CLIPPED względem near plane,
     * a dopiero później projektowany.
     */

    textured_triangle_clipped(
        camera,

        a,
        b,
        c,

        u0,v1,
        u1,v1,
        u1,v0,

        brightness
    );


    textured_triangle_clipped(
        camera,

        a,
        c,
        d,

        u0,v1,
        u1,v0,
        u0,v0,

        brightness
    );
}



/*
 * ============================================================
 * CHUNK MESH CACHE
 * ============================================================
 */

static ChunkMesh
    g_chunk_meshes[WORLD_CHUNK_COUNT];


/*
 * ============================================================
 * WORLD-SPACE FACE CULLING
 * ============================================================
 *
 * Chunk mesh przechowuje tylko exposed faces, ale nie oznacza
 * to jeszcze, że kamera może zobaczyć daną stronę bloku.
 *
 * Ten test kosztuje kilka porównań i odbywa się PRZED:
 *
 *   - atlas_uv
 *   - 4 x projection
 *   - perspective divide
 *   - STQ
 *   - GS submission
 *
 * Dzięki temu nie wykonujemy kosztownej pracy dla powierzchni
 * odwróconych od kamery.
 */

static int face_visible_from_camera(
    const Camera *camera,
    const ChunkMeshFace *face
)
{
    float cx;
    float cy;
    float cz;

    float nx;
    float ny;
    float nz;

    float vx;
    float vy;
    float vz;

    float dot;


    /*
     * Środek konkretnej ściany voxela.
     */
    cx = (float)face->x + 0.5f;
    cy = (float)face->y + 0.5f;
    cz = (float)face->z + 0.5f;


    nx = 0.0f;
    ny = 0.0f;
    nz = 0.0f;


    switch (face->face) {

        case MESH_FACE_TOP:
            cy += 0.5f;
            ny = 1.0f;
            break;

        case MESH_FACE_BOTTOM:
            cy -= 0.5f;
            ny = -1.0f;
            break;

        case MESH_FACE_SOUTH:
            cz += 0.5f;
            nz = 1.0f;
            break;

        case MESH_FACE_NORTH:
            cz -= 0.5f;
            nz = -1.0f;
            break;

        case MESH_FACE_EAST:
            cx += 0.5f;
            nx = 1.0f;
            break;

        case MESH_FACE_WEST:
            cx -= 0.5f;
            nx = -1.0f;
            break;

        default:
            return 0;
    }


    /*
     * Wektor:
     *
     *     face -> camera
     */
    vx = camera->x - cx;
    vy = camera->y - cy;
    vz = camera->z - cz;


    /*
     * Jeżeli kamera znajduje się po stronie normalnej
     * powierzchni, front face może być widoczny.
     */
    dot =
        vx * nx +
        vy * ny +
        vz * nz;


    /*
     * Mała tolerancja jest ważna przy kamerze bardzo
     * blisko powierzchni / zaglądaniu do wykopanej dziury.
     */
    return dot >= -0.05f;
}

static void render_cached_face(
    Camera *camera,
    const ChunkMeshFace *face
)
{
    float x0;
    float x1;

    float y0;
    float y1;

    float z0;
    float z1;

    BlockID type;


    /*
     * Najtańszy możliwy reject.
     *
     * Jeżeli kamera znajduje się po tylnej stronie płaszczyzny,
     * nie ma sensu nawet projektować vertexów.
     */
    if (
        !face_visible_from_camera(
            camera,
            face
        )
    ) {
        return;
    }


    x0 = (float)face->x;
    x1 = x0 + 1.0f;

    y0 = (float)face->y;
    y1 = y0 + 1.0f;

    z0 = (float)face->z;
    z1 = z0 + 1.0f;

    type =
        (BlockID)face->block;

    switch (face->face) {

        case MESH_FACE_TOP:

            textured_quad(
                camera,

                (Vec3){x0,y1,z0},
                (Vec3){x0,y1,z1},
                (Vec3){x1,y1,z1},
                (Vec3){x1,y1,z0},

                block_texture(
                    type,
                    FACE_TOP
                ),

                128
            );

            break;


        case MESH_FACE_BOTTOM:

            textured_quad(
                camera,

                (Vec3){x0,y0,z1},
                (Vec3){x0,y0,z0},
                (Vec3){x1,y0,z0},
                (Vec3){x1,y0,z1},

                block_texture(
                    type,
                    FACE_BOTTOM
                ),

                60
            );

            break;


        case MESH_FACE_SOUTH:

            textured_quad(
                camera,

                (Vec3){x0,y0,z1},
                (Vec3){x1,y0,z1},
                (Vec3){x1,y1,z1},
                (Vec3){x0,y1,z1},

                block_texture(
                    type,
                    FACE_SOUTH
                ),

                105
            );

            break;


        case MESH_FACE_NORTH:

            textured_quad(
                camera,

                (Vec3){x1,y0,z0},
                (Vec3){x0,y0,z0},
                (Vec3){x0,y1,z0},
                (Vec3){x1,y1,z0},

                block_texture(
                    type,
                    FACE_NORTH
                ),

                105
            );

            break;


        case MESH_FACE_EAST:

            textured_quad(
                camera,

                (Vec3){x1,y0,z1},
                (Vec3){x1,y0,z0},
                (Vec3){x1,y1,z0},
                (Vec3){x1,y1,z1},

                block_texture(
                    type,
                    FACE_EAST
                ),

                82
            );

            break;


        case MESH_FACE_WEST:

            textured_quad(
                camera,

                (Vec3){x0,y0,z0},
                (Vec3){x0,y0,z1},
                (Vec3){x0,y1,z1},
                (Vec3){x0,y1,z0},

                block_texture(
                    type,
                    FACE_WEST
                ),

                82
            );

            break;

        default:
            break;
    }
}



static void draw_block(
    World *world,
    Camera *camera,

    int x,
    int y,
    int z,

    BlockID type
)
{
    float x0;
    float x1;

    float y0;
    float y1;

    float z0;
    float z1;

    x0 = (float)x;
    x1 = x0 + 1.0f;

    y0 = (float)y;
    y1 = y0 + 1.0f;

    z0 = (float)z;
    z1 = z0 + 1.0f;


    /*
     * TOP
     * najjaśniejsza
     */
    if (
        world_get_block(
            world,
            x,
            y + 1,
            z
        ) == BLOCK_AIR
    ) {
        textured_quad(
            camera,

            (Vec3){x0,y1,z0},
            (Vec3){x0,y1,z1},
            (Vec3){x1,y1,z1},
            (Vec3){x1,y1,z0},

            block_texture(
                type,
                FACE_TOP
            ),

            128
        );
    }


    /*
     * SOUTH +Z
     */
    if (
        world_get_block(
            world,
            x,
            y,
            z + 1
        ) == BLOCK_AIR
    ) {
        textured_quad(
            camera,

            (Vec3){x0,y0,z1},
            (Vec3){x1,y0,z1},
            (Vec3){x1,y1,z1},
            (Vec3){x0,y1,z1},

            block_texture(
                type,
                FACE_SOUTH
            ),

            105
        );
    }


    /*
     * NORTH -Z
     */
    if (
        world_get_block(
            world,
            x,
            y,
            z - 1
        ) == BLOCK_AIR
    ) {
        textured_quad(
            camera,

            (Vec3){x1,y0,z0},
            (Vec3){x0,y0,z0},
            (Vec3){x0,y1,z0},
            (Vec3){x1,y1,z0},

            block_texture(
                type,
                FACE_NORTH
            ),

            105
        );
    }


    /*
     * EAST +X
     */
    if (
        world_get_block(
            world,
            x + 1,
            y,
            z
        ) == BLOCK_AIR
    ) {
        textured_quad(
            camera,

            (Vec3){x1,y0,z1},
            (Vec3){x1,y0,z0},
            (Vec3){x1,y1,z0},
            (Vec3){x1,y1,z1},

            block_texture(
                type,
                FACE_EAST
            ),

            82
        );
    }


    /*
     * WEST -X
     */
    if (
        world_get_block(
            world,
            x - 1,
            y,
            z
        ) == BLOCK_AIR
    ) {
        textured_quad(
            camera,

            (Vec3){x0,y0,z0},
            (Vec3){x0,y0,z1},
            (Vec3){x0,y1,z1},
            (Vec3){x0,y1,z0},

            block_texture(
                type,
                FACE_WEST
            ),

            82
        );
    }
}


void voxel_renderer_init(void)
{
}


void voxel_render_world(
    World *world,
    Camera *camera
)
{
    int ci;

    if (!world || !camera)
        return;


    /*
     * Cztery trigonometrie RAZ na całą klatkę.
     *
     * Wcześniej wykonywaliśmy je dla KAŻDEGO vertexa.
     */
    camera_cache_update(
        camera
    );


    for (
        ci = 0;
        ci < WORLD_CHUNK_COUNT;
        ++ci
    ) {

        Chunk *chunk;
        ChunkMesh *mesh;

        int fi;

        chunk =
            &world->chunks[ci];

        mesh =
            &g_chunk_meshes[ci];

        if (!chunk->generated)
            continue;

        /*
         * Build ONLY when terrain changed
         * or this cache belongs to another chunk.
         */
        if (
            chunk->mesh_dirty ||
            !mesh->valid ||
            mesh->chunk_x != chunk->chunk_x ||
            mesh->chunk_z != chunk->chunk_z
        ) {

            chunk_mesh_build(
                world,
                chunk,
                mesh
            );
        }

        /*
         * Frame-time work:
         *
         * iterate ONLY already-visible faces.
         */
        for (
            fi = 0;
            fi < mesh->face_count;
            ++fi
        ) {

            render_cached_face(
                camera,
                &mesh->faces[fi]
            );
        }
    }
}
