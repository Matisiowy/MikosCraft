#include <math.h>

#include "game.h"
#include "mikos_input.h"
#include "renderer.h"
#include "mikos_font.h"
#include "world.h"
#include "voxel_renderer.h"
#include "mikos_texture.h"
#include "player.h"
#include "interaction.h"

static const char *menu_items[] = {
    "PLAY GAME",
    "OPTIONS",
    "CREDITS"
};


static int surface_height(
    World *world,
    int x,
    int z
)
{
    int y;

    for (
        y = WORLD_Y - 1;
        y >= 0;
        y--
    ) {
        if (
            world_get_block(
                world,
                x,
                y,
                z
            ) != BLOCK_AIR
        ) {
            return y;
        }
    }

    return 0;
}


static void reset_player(
    Game *game
)
{
    int h;

    h =
        surface_height(
            &game->world,
            4,
            4
        );

    /*
     * Top bloku h kończy się na h+1.
     *
     * Player.y oznacza poziom stóp.
     */

    player_init(
        &game->player,
        4.5f,
        (float)h + 1.01f,
        4.5f
    );

    player_sync_camera(
        &game->player,
        &game->camera
    );
}


static void render_logo(void)
{
    font_draw_centered(
        "MIKOSCRAFT",
        323,74,7,
        20,20,20
    );

    font_draw_centered(
        "MIKOSCRAFT",
        320,70,7,
        90,210,95
    );

    font_draw_centered(
        "PLAYSTATION 2 EDITION",
        320,130,2,
        170,175,180
    );
}


static void render_button(
    const char *text,
    float y,
    int selected
)
{
    if (selected) {

        renderer_rect(
            167,y-3,
            306,50,
            105,190,105
        );

        renderer_rect(
            171,y+1,
            298,42,
            42,54,43
        );
    }

    else {

        renderer_rect(
            167,y-3,
            306,50,
            75,78,82
        );

        renderer_rect(
            171,y+1,
            298,42,
            36,39,43
        );
    }

    font_draw_centered(
        text,
        322,
        y+14,
        3,
        15,15,15
    );

    font_draw_centered(
        text,
        320,
        y+12,
        3,

        selected ? 245 : 210,
        selected ? 255 : 210,
        selected ? 245 : 210
    );
}


static void render_main_menu(
    Game *game
)
{
    int i;

    render_logo();

    for (i=0;i<3;i++) {

        render_button(
            menu_items[i],
            190+i*62,
            game->menu_selection==i
        );
    }

    font_draw_centered(
        "DPAD - MOVE    X - SELECT",
        320,400,2,
        130,135,140
    );

    font_draw_text(
        "MIKOSCRAFT PRE-ALPHA",
        12,480,1,
        95,100,105
    );
}


static void render_world_select(void)
{
    font_draw_centered(
        "SELECT WORLD",
        320,55,5,
        235,235,235
    );

    renderer_rect(
        75,115,
        490,210,
        45,48,52
    );

    renderer_rect(
        80,120,
        480,200,
        24,27,30
    );

    font_draw_centered(
        "NO WORLDS FOUND",
        320,190,3,
        155,160,165
    );

    font_draw_centered(
        "CREATE NEW WORLD",
        320,355,3,
        235,245,235
    );

    font_draw_centered(
        "X - CREATE    O - BACK",
        320,410,2,
        130,135,140
    );
}


static void render_create_world(void)
{
    font_draw_centered(
        "CREATE NEW WORLD",
        320,65,5,
        235,235,235
    );

    font_draw_centered(
        "WORLD NAME",
        320,165,2,
        160,165,170
    );

    renderer_rect(
        150,200,
        340,45,
        55,58,62
    );

    font_draw_centered(
        "NEW WORLD",
        320,215,2,
        235,235,235
    );

    renderer_rect(
        155,292,
        330,48,
        75,145,75
    );

    renderer_rect(
        159,296,
        322,40,
        37,54,38
    );

    font_draw_centered(
        "GENERATE WORLD",
        320,309,3,
        235,255,235
    );

    font_draw_centered(
        "X - GENERATE    O - BACK",
        320,410,2,
        130,135,140
    );
}


static void render_playing(
    Game *game
)
{
    voxel_render_world(
        &game->world,
        &game->camera
    );

    /*
     * crosshair
     */

    renderer_rect(
        315,255,
        11,2,
        255,255,255
    );

    renderer_rect(
        320,250,
        2,12,
        255,255,255
    );
}


void game_init(
    Game *game
)
{
    game->state =
        GAME_STATE_MAIN_MENU;

    game->menu_selection = 0;

    game->running = 1;

    world_generate(
        &game->world,
        2137
    );

    reset_player(game);

    mikos_texture_init();

    voxel_renderer_init();
}


void game_update(
    Game *game
)
{
    if (
        game->state ==
        GAME_STATE_MAIN_MENU
    ) {

        if (
            g_input.pressed &
            INPUT_UP
        ) {

            game->menu_selection--;

            if (
                game->menu_selection < 0
            ) {
                game->menu_selection=2;
            }
        }

        if (
            g_input.pressed &
            INPUT_DOWN
        ) {

            game->menu_selection++;

            if (
                game->menu_selection > 2
            ) {
                game->menu_selection=0;
            }
        }

        if (
            g_input.pressed &
            INPUT_CROSS
        ) {

            if (
                game->menu_selection==0
            ) {
                game->state =
                    GAME_STATE_WORLD_SELECT;
            }
        }
    }


    else if (
        game->state ==
        GAME_STATE_WORLD_SELECT
    ) {

        if (
            g_input.pressed &
            INPUT_CIRCLE
        ) {

            game->state =
                GAME_STATE_MAIN_MENU;

            game->menu_selection=0;
        }

        if (
            g_input.pressed &
            INPUT_CROSS
        ) {

            game->state =
                GAME_STATE_CREATE_WORLD;
        }
    }


    else if (
        game->state ==
        GAME_STATE_CREATE_WORLD
    ) {

        if (
            g_input.pressed &
            INPUT_CIRCLE
        ) {

            game->state =
                GAME_STATE_WORLD_SELECT;
        }

        if (
            g_input.pressed &
            INPUT_CROSS
        ) {

            world_generate(
                &game->world,
                2137
            );

            reset_player(game);

            game->state =
                GAME_STATE_PLAYING;
        }
    }


    else if (
        game->state ==
        GAME_STATE_PLAYING
    ) {

        /*
         * Prawdziwy player controller.
         *
         * Kamera nie porusza się już samodzielnie.
         */

        player_update(
            &game->player,
            &game->world,
            &game->camera
        );

        /*
         * Block interaction:
         *
         * R1 = break
         * L1 = place
         */
        interaction_update(
            &game->world,
            &game->player
        );


        /*
         * Circle wraca do menu.
         */

        if (
            g_input.pressed &
            INPUT_CIRCLE
        ) {

            game->state =
                GAME_STATE_MAIN_MENU;

            game->menu_selection =
                0;
        }
    }

}


void game_render(
    Game *game
)
{
    switch (
        game->state
    ) {

        case GAME_STATE_MAIN_MENU:

            render_main_menu(game);
            break;


        case GAME_STATE_WORLD_SELECT:

            render_world_select();
            break;


        case GAME_STATE_CREATE_WORLD:

            render_create_world();
            break;


        case GAME_STATE_PLAYING:

            render_playing(game);
            break;


        default:
            break;
    }
}
