#ifndef MIKOSCRAFT_PLAYER_H
#define MIKOSCRAFT_PLAYER_H

#include "world.h"
#include "voxel_renderer.h"

/*
 * Minecraft-ish player dimensions.
 *
 * x/z = środek gracza
 * y   = stopy
 */
#define PLAYER_WIDTH       0.60f
#define PLAYER_HALF_WIDTH  0.30f
#define PLAYER_HEIGHT      1.80f
#define PLAYER_EYE_HEIGHT  1.62f

typedef struct {

    float x;
    float y;
    float z;

    float velocity_x;
    float velocity_y;
    float velocity_z;

    float yaw;
    float pitch;

    int on_ground;
    int running;
    int moving;

    /*
     * Do camera bob.
     */
    float walk_time;
    float bob_amount;

} Player;

void player_init(
    Player *player,
    float x,
    float y,
    float z
);

void player_update(
    Player *player,
    World *world,
    Camera *camera
);

void player_sync_camera(
    const Player *player,
    Camera *camera
);

#endif
