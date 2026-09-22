# MikosCraft

A Minecraft-inspired voxel sandbox built from scratch for the Sony PlayStation 2.

MikosCraft uses PS2SDK and gsKit directly. TyraCraft / Tyra Engine are used only as technical references and are not dependencies.

## Current status

Early development / pre-alpha.

Currently implemented:

- PlayStation 2 native ELF
- gsKit renderer
- textured voxel terrain
- Minecraft-style texture atlas support
- procedural seeded terrain
- 8x32x8 chunks
- 3x3 resident chunk world
- cached exposed-face chunk meshes
- world-space face culling
- camera transform cache
- perspective-correct STQ texture mapping
- near-plane triangle clipping
- DualShock 2 analog controls
- player movement
- sprinting
- jumping and gravity
- AABB block collision
- block raycasting
- hold-to-mine interaction
- block placement
- dynamic chunk mesh rebuilding
- basic menus and HUD
- stable 50 FPS target

## Controls

- Left analog stick - Move
- Right analog stick - Look
- Cross - Jump
- Square - Sprint
- R2 - Mine block
- L2 - Place block
- Circle - Back / menu

## Planned

- block breaking crack animation
- item drops
- 9-slot hotbar
- inventory
- crafting
- tools and mining speeds
- larger dynamic world
- chunk streaming
- improved terrain generation
- save/load
- lighting
- transparent blocks
- water
- mobs
- audio/music
- renderer batching / further PS2 optimization

## Building

Requires:

- PS2DEV
- PS2SDK
- gsKit

Build with:

    make -j4

The resulting executable is:

    MIKOSCRAFT.ELF

## Assets

Minecraft assets are not included in this repository.

For development, MikosCraft can use assets extracted from a legally owned Minecraft installation. These assets remain property of their respective copyright holders.

## Technical notes

The renderer is custom and based directly on PS2SDK/gsKit.

Current voxel rendering pipeline:

    World
      -> Chunks
      -> Exposed-face ChunkMesh
      -> World-space face culling
      -> Camera-space transform
      -> Near-plane clipping
      -> Perspective projection
      -> Perspective-correct STQ
      -> PlayStation 2 GS

Future renderer work will focus on larger geometry batches, chunk frustum culling and PS2-specific DMA/VU1 optimization.

## License

Source code licensing to be decided.

MikosCraft is an independent hobby project and is not affiliated with Mojang Studios or Microsoft.
