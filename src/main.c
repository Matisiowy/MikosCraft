#include <kernel.h>
#include <sifrpc.h>

#include "game.h"
#include "block.h"
#include "mikos_input.h"
#include "renderer.h"

int main(int argc, char *argv[])
{
    block_registry_init();
    Game game;

    SifInitRpc(0);

    if (renderer_init() < 0)
        SleepThread();

    if (input_init() < 0)
        SleepThread();

    game_init(&game);

    while (game.running) {
        input_update();
        game_update(&game);

        renderer_begin();
        game_render(&game);
        renderer_end();
    }

    SleepThread();

    return 0;
}
