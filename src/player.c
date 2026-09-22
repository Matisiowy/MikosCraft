#include <math.h>

#include "player.h"
#include "mikos_input.h"
#include "block.h"

/*
 * Gra chodzi PAL 50 Hz.
 *
 * Na razie fizyka fixed-step per frame.
 * Później możemy zrobić prawdziwe deltaTime.
 */

#define WALK_SPEED       0.070f
#define RUN_SPEED        0.105f

#define ACCELERATION     0.020f
#define DECELERATION     0.030f

#define GRAVITY         -0.018f
#define JUMP_VELOCITY    0.235f
#define TERMINAL_SPEED  -0.42f

#define LOOK_SPEED_X     0.040f
#define LOOK_SPEED_Y     0.034f

#define COLLISION_STEP   0.025f
#define EPSILON          0.001f

#define PI_F             3.14159265358979323846f


static float approach(
    float current,
    float target,
    float amount
)
{
    if (current < target) {

        current += amount;

        if (current > target)
            current = target;
    }

    else if (current > target) {

        current -= amount;

        if (current < target)
            current = target;
    }

    return current;
}


/*
 * ------------------------------------------------------------
 * AABB collision
 * ------------------------------------------------------------
 */

static int collides(
    const World *world,
    float x,
    float y,
    float z
)
{
    int min_x;
    int max_x;

    int min_y;
    int max_y;

    int min_z;
    int max_z;

    int bx;
    int by;
    int bz;

    min_x =
        (int)floorf(
            x -
            PLAYER_HALF_WIDTH +
            EPSILON
        );

    max_x =
        (int)floorf(
            x +
            PLAYER_HALF_WIDTH -
            EPSILON
        );

    min_y =
        (int)floorf(
            y +
            EPSILON
        );

    max_y =
        (int)floorf(
            y +
            PLAYER_HEIGHT -
            EPSILON
        );

    min_z =
        (int)floorf(
            z -
            PLAYER_HALF_WIDTH +
            EPSILON
        );

    max_z =
        (int)floorf(
            z +
            PLAYER_HALF_WIDTH -
            EPSILON
        );

    for (bx = min_x; bx <= max_x; bx++) {

        for (by = min_y; by <= max_y; by++) {

            /*
             * Dół/góra świata traktujemy jak ścianę.
             */
            if (
                by < 0 ||
                by >= WORLD_Y
            ) {
                return 1;
            }

            for (
                bz = min_z;
                bz <= max_z;
                bz++
            ) {

                BlockID block;

                block =
                    world_get_block(
                        world,
                        bx,
                        by,
                        bz
                    );

                if (
                    block != BLOCK_AIR &&
                    block_is_solid(block)
                ) {
                    return 1;
                }
            }
        }
    }

    return 0;
}


/*
 * Małe kroki = brak przelatywania przez voxel
 * przy większej velocity.
 */

static void move_x(
    Player *player,
    const World *world,
    float amount
)
{
    float remaining;

    remaining = amount;

    while (
        fabsf(remaining) >
        0.00001f
    ) {

        float step;

        if (remaining > COLLISION_STEP)
            step = COLLISION_STEP;

        else if (remaining < -COLLISION_STEP)
            step = -COLLISION_STEP;

        else
            step = remaining;

        if (
            collides(
                world,
                player->x + step,
                player->y,
                player->z
            )
        ) {

            player->velocity_x =
                0.0f;

            return;
        }

        player->x += step;
        remaining -= step;
    }
}


static void move_z(
    Player *player,
    const World *world,
    float amount
)
{
    float remaining;

    remaining = amount;

    while (
        fabsf(remaining) >
        0.00001f
    ) {

        float step;

        if (remaining > COLLISION_STEP)
            step = COLLISION_STEP;

        else if (remaining < -COLLISION_STEP)
            step = -COLLISION_STEP;

        else
            step = remaining;

        if (
            collides(
                world,
                player->x,
                player->y,
                player->z + step
            )
        ) {

            player->velocity_z =
                0.0f;

            return;
        }

        player->z += step;
        remaining -= step;
    }
}


static void move_y(
    Player *player,
    const World *world,
    float amount
)
{
    float remaining;

    remaining = amount;

    while (
        fabsf(remaining) >
        0.00001f
    ) {

        float step;

        if (remaining > COLLISION_STEP)
            step = COLLISION_STEP;

        else if (remaining < -COLLISION_STEP)
            step = -COLLISION_STEP;

        else
            step = remaining;

        if (
            collides(
                world,
                player->x,
                player->y + step,
                player->z
            )
        ) {

            if (step < 0.0f)
                player->on_ground = 1;

            player->velocity_y =
                0.0f;

            return;
        }

        player->y += step;
        remaining -= step;
    }
}


/*
 * ------------------------------------------------------------
 * Camera
 * ------------------------------------------------------------
 */

void player_sync_camera(
    const Player *player,
    Camera *camera
)
{
    float bob_y;

    bob_y = 0.0f;

    /*
     * Lekki Minecraft/TyraCraft-like head bob.
     * Tylko podczas ruchu po ziemi.
     */
    if (
        player->moving &&
        player->on_ground
    ) {

        bob_y =
            sinf(
                player->walk_time * 2.0f
            ) *
            player->bob_amount;
    }

    camera->x =
        player->x;

    camera->y =
        player->y +
        PLAYER_EYE_HEIGHT +
        bob_y;

    camera->z =
        player->z;

    camera->yaw =
        player->yaw;

    camera->pitch =
        player->pitch;
}


/*
 * ------------------------------------------------------------
 * Spawn
 * ------------------------------------------------------------
 */

void player_init(
    Player *player,
    float x,
    float y,
    float z
)
{
    player->x = x;
    player->y = y;
    player->z = z;

    player->velocity_x = 0.0f;
    player->velocity_y = 0.0f;
    player->velocity_z = 0.0f;

    player->yaw = 0.0f;
    player->pitch = 0.0f;

    player->on_ground = 0;
    player->running = 0;
    player->moving = 0;

    player->walk_time = 0.0f;
    player->bob_amount = 0.018f;
}


/*
 * ------------------------------------------------------------
 * Gameplay
 * ------------------------------------------------------------
 */

void player_update(
    Player *player,
    World *world,
    Camera *camera
)
{
    float input_forward;
    float input_strafe;

    float desired_x;
    float desired_z;

    float input_length;
    float target_speed;

    /*
     * ========================================================
     * CAMERA / RIGHT STICK
     * ========================================================
     */

    if (g_input.analog_valid) {

        player->yaw +=
            g_input.right_x *
            LOOK_SPEED_X;

        /*
         * DualShock Y:
         * góra = wartość ujemna
         * dół  = wartość dodatnia
         *
         * Odwracamy dla naturalnego sterowania kamerą:
         * stick góra -> patrzymy w górę.
         */
        player->pitch -=
            g_input.right_y *
            LOOK_SPEED_Y;
    }

    /*
     * Debug fallback:
     * D-pad L/R obraca.
     */
    else {

        if (g_input.held & INPUT_LEFT)
            player->yaw -= 0.035f;

        if (g_input.held & INPUT_RIGHT)
            player->yaw += 0.035f;
    }


    if (player->pitch < -1.45f)
        player->pitch = -1.45f;

    if (player->pitch > 1.45f)
        player->pitch = 1.45f;


    /*
     * Nie pozwalamy yaw rosnąć do milionów.
     */
    if (player->yaw > PI_F * 2.0f)
        player->yaw -= PI_F * 2.0f;

    if (player->yaw < -PI_F * 2.0f)
        player->yaw += PI_F * 2.0f;


    /*
     * ========================================================
     * LEFT STICK
     * ========================================================
     */

    input_forward = 0.0f;
    input_strafe = 0.0f;

    if (g_input.analog_valid) {

        input_forward =
            -g_input.left_y;

        input_strafe =
            g_input.left_x;
    }

    else {

        if (g_input.held & INPUT_UP)
            input_forward += 1.0f;

        if (g_input.held & INPUT_DOWN)
            input_forward -= 1.0f;
    }


    /*
     * Normalizacja diagonala.
     */
    input_length =
        sqrtf(
            input_forward * input_forward +
            input_strafe * input_strafe
        );

    if (input_length > 1.0f) {

        input_forward /=
            input_length;

        input_strafe /=
            input_length;

        input_length = 1.0f;
    }


    player->moving =
        input_length > 0.02f;


    /*
     * TyraCraft ma osobny running state.
     *
     * U nas Square = sprint.
     */
    player->running =
        (
            g_input.held &
            INPUT_SQUARE
        ) != 0;


    target_speed =
        player->running
        ? RUN_SPEED
        : WALK_SPEED;


    /*
     * Ruch względem kierunku kamery.
     */
    desired_x =
        (
            sinf(player->yaw) *
            input_forward
            +
            cosf(player->yaw) *
            input_strafe
        ) *
        target_speed;

    desired_z =
        (
            cosf(player->yaw) *
            input_forward
            -
            sinf(player->yaw) *
            input_strafe
        ) *
        target_speed;


    /*
     * ========================================================
     * ACCELERATION / FRICTION
     * ========================================================
     *
     * To daje dużo przyjemniejsze sterowanie niż
     * velocity = stick * speed.
     */

    if (player->moving) {

        player->velocity_x =
            approach(
                player->velocity_x,
                desired_x,
                ACCELERATION
            );

        player->velocity_z =
            approach(
                player->velocity_z,
                desired_z,
                ACCELERATION
            );
    }

    else {

        player->velocity_x =
            approach(
                player->velocity_x,
                0.0f,
                DECELERATION
            );

        player->velocity_z =
            approach(
                player->velocity_z,
                0.0f,
                DECELERATION
            );
    }


    /*
     * ========================================================
     * JUMP
     * ========================================================
     */

    if (
        (
            g_input.pressed &
            INPUT_CROSS
        ) &&
        player->on_ground
    ) {

        player->velocity_y =
            JUMP_VELOCITY;

        player->on_ground =
            0;
    }


    /*
     * ========================================================
     * GRAVITY
     * ========================================================
     */

    player->velocity_y +=
        GRAVITY;

    if (
        player->velocity_y <
        TERMINAL_SPEED
    ) {
        player->velocity_y =
            TERMINAL_SPEED;
    }


    /*
     * ========================================================
     * COLLISION
     * ========================================================
     *
     * X i Z osobno = sliding po ścianach.
     */

    move_x(
        player,
        world,
        player->velocity_x
    );

    move_z(
        player,
        world,
        player->velocity_z
    );

    /*
     * Y na końcu.
     */
    player->on_ground = 0;

    move_y(
        player,
        world,
        player->velocity_y
    );


    /*
     * ========================================================
     * CAMERA BOB
     * ========================================================
     */

    if (
        player->moving &&
        player->on_ground
    ) {

        player->walk_time +=
            player->running
            ? 0.28f
            : 0.21f;
    }


    /*
     * ========================================================
     * CAMERA FOLLOW
     * ========================================================
     */

    player_sync_camera(
        player,
        camera
    );
}
