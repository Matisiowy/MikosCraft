#include <kernel.h>
#include <gsKit.h>
#include <string.h>

#include "renderer.h"
#include "mikos_texture.h"

extern u32 terrain_atlas_pixels[256 * 256];

GSTEXTURE g_terrain_texture;


int mikos_texture_init(void)
{
    u32 texture_size;

    /*
     * Bardzo ważne:
     * wyzeruj całą strukturę przed konfiguracją.
     */
    memset(
        &g_terrain_texture,
        0,
        sizeof(GSTEXTURE)
    );


    /* =====================================================
     * MIKOSCRAFT TERRAIN ATLAS
     * ===================================================== */

    g_terrain_texture.Width  = 256;
    g_terrain_texture.Height = 256;

    g_terrain_texture.PSM =
        GS_PSM_CT32;

    g_terrain_texture.Mem =
        terrain_atlas_pixels;

    g_terrain_texture.Clut =
        NULL;

    g_terrain_texture.Filter =
        GS_FILTER_NEAREST;

    g_terrain_texture.Delayed =
        0;


    /* =====================================================
     * VRAM
     *
     * Vram = 0 NIE jest automatyczną alokacją.
     * Rezerwujemy prawdziwy obszar VRAM dla atlasu.
     * ===================================================== */

    texture_size =
        gsKit_texture_size(
            g_terrain_texture.Width,
            g_terrain_texture.Height,
            g_terrain_texture.PSM
        );

    g_terrain_texture.Vram =
        gsKit_vram_alloc(
            g_gs,
            texture_size,
            GSKIT_ALLOC_USERBUFFER
        );


    /*
     * Brak CLUT, ponieważ używamy RGBA32,
     * a nie tekstury indeksowanej.
     */

    g_terrain_texture.VramClut =
        0;


    /* =====================================================
     * UPLOAD RAM -> GS VRAM
     * ===================================================== */

    gsKit_texture_upload(
        g_gs,
        &g_terrain_texture
    );


    /*
     * Pixel-art Minecrafta:
     * żadnego bilinear filtering.
     */

    gsKit_set_texfilter(
        g_gs,
        GS_FILTER_NEAREST
    );

    return 0;
}
