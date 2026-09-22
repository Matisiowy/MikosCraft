#ifndef MIKOSCRAFT_RENDERER_H
#define MIKOSCRAFT_RENDERER_H

#include <gsKit.h>

extern GSGLOBAL *g_gs;

int renderer_init(void);
void renderer_begin(void);
void renderer_end(void);

void renderer_rect(
    float x,
    float y,
    float w,
    float h,
    int r,
    int g,
    int b
);

#endif
