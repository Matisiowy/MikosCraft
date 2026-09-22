#ifndef MIKOSCRAFT_VOXEL_RENDERER_H
#define MIKOSCRAFT_VOXEL_RENDERER_H

#include "world.h"

typedef struct {
    float x;
    float y;
    float z;

    float yaw;
    float pitch;
} Camera;

void voxel_renderer_init(void);

void voxel_render_world(
    World *world,
    Camera *camera
);

#endif
