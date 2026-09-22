#ifndef MIKOSCRAFT_GAME_H
#define MIKOSCRAFT_GAME_H

#include "world.h"
#include "voxel_renderer.h"
#include "player.h"

typedef enum {
    GAME_STATE_MAIN_MENU,
    GAME_STATE_WORLD_SELECT,
    GAME_STATE_CREATE_WORLD,
    GAME_STATE_GENERATING,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED
} GameState;

typedef struct {
    GameState state;

    int menu_selection;
    int running;

    World world;
    Player player;
    Camera camera;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_render(Game *game);

#endif
