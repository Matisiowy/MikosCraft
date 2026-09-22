#ifndef MIKOSCRAFT_FONT_H
#define MIKOSCRAFT_FONT_H

void font_draw_text(
    const char *text,
    float x,
    float y,
    float scale,
    int r,
    int g,
    int b
);

float font_text_width(const char *text, float scale);

void font_draw_centered(
    const char *text,
    float center_x,
    float y,
    float scale,
    int r,
    int g,
    int b
);

#endif
