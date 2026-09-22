#include <math.h>

#include "interaction.h"
#include "mikos_input.h"
#include "block.h"


#define RAYCAST_REACH 5.0f
#define RAYCAST_STEP  0.05f


static int floor_int(float value)
{
    int i;

    i = (int)value;

    if (
        value < 0.0f &&
        value != (float)i
    ) {
        i--;
    }

    return i;
}


/*
 * Czy blok, który chcemy postawić, wszedłby
 * w AABB gracza?
 */
static int block_intersects_player(
    const Player *player,
    int bx,
    int by,
    int bz
)
{
    float player_min_x;
    float player_max_x;

    float player_min_y;
    float player_max_y;

    float player_min_z;
    float player_max_z;

    float block_min_x;
    float block_max_x;

    float block_min_y;
    float block_max_y;

    float block_min_z;
    float block_max_z;


    player_min_x =
        player->x - PLAYER_HALF_WIDTH;

    player_max_x =
        player->x + PLAYER_HALF_WIDTH;

    player_min_y =
        player->y;

    player_max_y =
        player->y + PLAYER_HEIGHT;

    player_min_z =
        player->z - PLAYER_HALF_WIDTH;

    player_max_z =
        player->z + PLAYER_HALF_WIDTH;


    block_min_x = (float)bx;
    block_max_x = (float)bx + 1.0f;

    block_min_y = (float)by;
    block_max_y = (float)by + 1.0f;

    block_min_z = (float)bz;
    block_max_z = (float)bz + 1.0f;


    if (
        player_max_x <= block_min_x ||
        player_min_x >= block_max_x
    ) {
        return 0;
    }

    if (
        player_max_y <= block_min_y ||
        player_min_y >= block_max_y
    ) {
        return 0;
    }

    if (
        player_max_z <= block_min_z ||
        player_min_z >= block_max_z
    ) {
        return 0;
    }

    return 1;
}


BlockTarget interaction_raycast(
    const World *world,
    const Player *player,
    float max_distance
)
{
    BlockTarget result;

    float origin_x;
    float origin_y;
    float origin_z;

    float dir_x;
    float dir_y;
    float dir_z;

    float horizontal;

    float distance;

    int previous_x;
    int previous_y;
    int previous_z;


    result.hit = 0;

    result.x = 0;
    result.y = 0;
    result.z = 0;

    result.place_x = 0;
    result.place_y = 0;
    result.place_z = 0;

    result.block = BLOCK_AIR;


    if (
        !world ||
        !player
    ) {
        return result;
    }


    /*
     * Kamera jest przy oczach gracza.
     */
    origin_x = player->x;
    origin_y = player->y + PLAYER_EYE_HEIGHT;
    origin_z = player->z;


    /*
     * Kierunek musi odpowiadać temu samemu układowi
     * yaw/pitch, którego używa renderer.
     *
     * yaw = 0 -> +Z
     */

    horizontal =
        cosf(player->pitch);

    dir_x =
        sinf(player->yaw) *
        horizontal;

    dir_y =
        sinf(player->pitch);

    dir_z =
        cosf(player->yaw) *
        horizontal;


    previous_x =
        floor_int(origin_x);

    previous_y =
        floor_int(origin_y);

    previous_z =
        floor_int(origin_z);


    for (
        distance = 0.0f;
        distance <= max_distance;
        distance += RAYCAST_STEP
    ) {

        float px;
        float py;
        float pz;

        int bx;
        int by;
        int bz;

        BlockID block;


        px =
            origin_x +
            dir_x * distance;

        py =
            origin_y +
            dir_y * distance;

        pz =
            origin_z +
            dir_z * distance;


        bx = floor_int(px);
        by = floor_int(py);
        bz = floor_int(pz);


        /*
         * Nie sprawdzamy wielokrotnie tego samego voxela.
         */
        if (
            bx == previous_x &&
            by == previous_y &&
            bz == previous_z &&
            distance > 0.0f
        ) {
            continue;
        }


        block =
            world_get_block(
                world,
                bx,
                by,
                bz
            );


        if (
            block != BLOCK_AIR
        ) {

            result.hit = 1;

            result.x = bx;
            result.y = by;
            result.z = bz;

            result.place_x =
                previous_x;

            result.place_y =
                previous_y;

            result.place_z =
                previous_z;

            result.block =
                block;

            return result;
        }


        previous_x = bx;
        previous_y = by;
        previous_z = bz;
    }


    return result;
}



/*
 * ============================================================
 * MINING STATE
 * ============================================================
 *
 * PS2 chodzi w 50 Hz, więc liczymy postęp per-frame.
 *
 * hardness:
 *   1 = szybko
 *   2 = normalnie
 *   3 = wolniej
 *   4 = jeszcze wolniej
 *
 * Później ten system dostanie:
 * - tool speed
 * - efficiency
 * - creative instant break
 * - crack stages
 */

static int mining_active = 0;

static int mining_x = 0;
static int mining_y = 0;
static int mining_z = 0;

static float mining_progress = 0.0f;


static void mining_reset(void)
{
    mining_active = 0;

    mining_x = 0;
    mining_y = 0;
    mining_z = 0;

    mining_progress = 0.0f;
}


static float mining_speed_for_block(
    BlockID block
)
{
    const BlockDefinition *definition;

    definition =
        block_get_definition(block);

    if (
        !definition ||
        !definition->breakable
    ) {
        return 0.0f;
    }

    /*
     * Docelowo narzędzia będą modyfikowały ten czas.
     *
     * Przy 50 FPS:
     *
     * hardness 1 -> ~0.25 s
     * hardness 2 -> ~0.50 s
     * hardness 3 -> ~0.75 s
     * hardness 4 -> ~1.00 s
     */

    if (definition->hardness == 0)
        return 1.0f;

    return
        1.0f /
        (
            (float)definition->hardness *
            12.5f
        );
}


void interaction_update(
    World *world,
    Player *player
)
{
    BlockTarget target;


    if (
        !world ||
        !player
    ) {
        mining_reset();
        return;
    }


    target =
        interaction_raycast(
            world,
            player,
            RAYCAST_REACH
        );


    /*
     * ========================================================
     * R2 = HOLD TO MINE
     * ========================================================
     */

    if (
        g_input.held &
        INPUT_R2
    ) {

        const BlockDefinition *definition;
        float speed;


        /*
         * Nie patrzymy na żaden blok.
         */
        if (!target.hit) {

            mining_reset();
            return;
        }


        definition =
            block_get_definition(
                target.block
            );


        /*
         * Bedrock / air / przyszłe bloki niezniszczalne.
         */
        if (
            !definition ||
            !definition->breakable
        ) {

            mining_reset();
            return;
        }


        /*
         * Zaczynamy niszczyć nowy blok albo gracz
         * przesunął celownik na inny voxel.
         */
        if (
            !mining_active ||

            mining_x != target.x ||
            mining_y != target.y ||
            mining_z != target.z
        ) {

            mining_active = 1;

            mining_x = target.x;
            mining_y = target.y;
            mining_z = target.z;

            mining_progress = 0.0f;
        }


        speed =
            mining_speed_for_block(
                target.block
            );


        mining_progress += speed;


        /*
         * Block broken.
         */
        if (
            mining_progress >= 1.0f
        ) {

            world_set_block(
                world,

                target.x,
                target.y,
                target.z,

                BLOCK_AIR
            );


            /*
             * TODO:
             * add block/item drop to inventory.
             */

            mining_reset();
        }


        /*
         * Kiedy kopiemy, nie wykonujemy place
         * w tej samej klatce.
         */
        return;
    }


    /*
     * Puszczenie R2 natychmiast kasuje postęp.
     */
    mining_reset();


    /*
     * ========================================================
     * L2 = PLACE
     * ========================================================
     *
     * Tymczasowo cobblestone.
     * Za chwilę będzie selected hotbar slot.
     */

    if (
        g_input.pressed &
        INPUT_L2
    ) {

        if (!target.hit)
            return;


        if (
            world_is_inside(
                world,

                target.place_x,
                target.place_y,
                target.place_z
            ) &&

            world_get_block(
                world,

                target.place_x,
                target.place_y,
                target.place_z
            ) == BLOCK_AIR &&

            !block_intersects_player(
                player,

                target.place_x,
                target.place_y,
                target.place_z
            )
        ) {

            world_set_block(
                world,

                target.place_x,
                target.place_y,
                target.place_z,

                BLOCK_COBBLESTONE
            );
        }
    }
}
