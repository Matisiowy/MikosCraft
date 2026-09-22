#include <kernel.h>
#include <gsKit.h>
#include <dmaKit.h>

#include "renderer.h"

GSGLOBAL *g_gs = NULL;


int renderer_init(void)
{
    dmaKit_init(
        D_CTRL_RELE_OFF,
        D_CTRL_MFD_OFF,
        D_CTRL_STS_UNSPEC,
        D_CTRL_STD_OFF,
        D_CTRL_RCYC_8,
        1 << DMA_CHANNEL_GIF
    );

    dmaKit_chan_init(
        DMA_CHANNEL_GIF
    );

    g_gs = gsKit_init_global();

    if (!g_gs)
        return -1;


    /*
     * FRAMEBUFFER
     */

    g_gs->PSM =
        GS_PSM_CT24;


    /*
     * DEPTH BUFFER
     */

    g_gs->PSMZ =
        GS_PSMZ_16S;

    g_gs->DoubleBuffering =
        GS_SETTING_ON;

    /*
     * MUSI być ON przed gsKit_init_screen().
     * Wtedy gsKit przydziela VRAM na Z-buffer.
     */

    g_gs->ZBuffering =
        GS_SETTING_ON;


    /*
     * Teraz dopiero inicjalizacja GS.
     */

    gsKit_init_screen(
        g_gs
    );


    /*
     * Oficjalny przykład cube robi dokładnie
     * takie włączenie testu głębokości.
     */

    gsKit_set_test(
        g_gs,
        GS_ZTEST_ON
    );


    /*
     * Pixel-art.
     */

    gsKit_set_texfilter(
        g_gs,
        GS_FILTER_NEAREST
    );


    gsKit_mode_switch(
        g_gs,
        GS_ONESHOT
    );

    return 0;
}


void renderer_begin(void)
{
    /*
     * Clear framebuffer/depth w standardowym
     * przepływie gsKit.
     */

    gsKit_clear(
        g_gs,
        GS_SETREG_RGBAQ(
            100,
            155,
            220,
            0x80,
            0x00
        )
    );


    /*
     * gsKit_clear tymczasowo zmienia test.
     * Ustawiamy jawnie stan wymagany przez świat 3D.
     */

    gsKit_set_test(
        g_gs,
        GS_ZTEST_ON
    );
}


void renderer_end(void)
{
    /*
     * Wyślij command queue do GS.
     */
    gsKit_queue_exec(
        g_gs
    );

    /*
     * Poczekaj / flip framebuffer.
     */
    gsKit_sync_flip(
        g_gs
    );

    /*
     * GS_ONESHOT:
     *
     * Po wykonaniu klatki czyścimy queue.
     * W przeciwnym przypadku stare primitive'y mogą
     * pozostać w kolejce i koszt kolejnych klatek
     * może rosnąć wraz z czasem działania gry.
     */
    gsKit_queue_reset(
        g_gs->Os_Queue
    );
}


void renderer_rect(
    float x,
    float y,
    float w,
    float h,
    int r,
    int g,
    int b
)
{
    /*
     * HUD powinien zawsze wygrać z geometrią.
     * Używamy wysokiej wartości Z.
     */

    gsKit_prim_sprite(
        g_gs,

        x,
        y,

        x + w,
        y + h,

        65535,

        GS_SETREG_RGBAQ(
            r,
            g,
            b,
            0x80,
            0x00
        )
    );
}
