#ifndef MIKOSCRAFT_INTERACTION_H
#define MIKOSCRAFT_INTERACTION_H

#include "world.h"
#include "player.h"

typedef struct {

    int hit;

    int x;
    int y;
    int z;

    /*
     * Ostatni pusty blok przed trafionym blokiem.
     * Tutaj możemy postawić nowy blok.
     */
    int place_x;
    int place_y;
    int place_z;

    BlockID block;

} BlockTarget;


/*
 * Raycast z oczu gracza.
 *
 * max_distance w blokach.
 */
BlockTarget interaction_raycast(
    const World *world,
    const Player *player,
    float max_distance
);


/*
 * Obsługa R1/L1.
 */
void interaction_update(
    World *world,
    Player *player
);

#endif
