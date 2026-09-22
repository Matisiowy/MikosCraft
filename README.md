<div align="center">

# 🟩 MIKOSCRAFT

### A voxel sandbox built from scratch for the PlayStation 2

**PS2SDK · gsKit · C · Emotion Engine · Graphics Synthesizer**

![Status](https://img.shields.io/badge/status-pre--alpha-orange)
![Platform](https://img.shields.io/badge/platform-PlayStation%202-003791)
![Language](https://img.shields.io/badge/language-C-blue)
![FPS](https://img.shields.io/badge/target-50%20FPS-brightgreen)

---

**MikosCraft** is an experimental Minecraft-inspired voxel sandbox written specifically for the **Sony PlayStation 2**.

It uses **PS2SDK and gsKit directly** and features a custom voxel renderer, procedural terrain, chunk meshing, player physics and block interaction.

No general-purpose game engine is used.

</div>

---

## 🎮 Current State

MikosCraft is currently in **early pre-alpha development**.

The game can already generate a voxel world, render textured chunks, move a player through the world with collision, and allow blocks to be mined and placed.

### Currently working

- 🌍 Seed-based procedural terrain
- 🧱 Block registry
- 📦 Chunk-based world storage
- ⚡ Cached exposed-face chunk meshes
- 🎨 Minecraft-style texture atlas support
- 🎥 First-person camera
- 🎮 DualShock 2 analog controls
- 🏃 Walking and sprinting
- 🦘 Jumping and gravity
- 💥 AABB player/world collision
- 🎯 Block raycasting
- ⛏️ Hold-to-mine block breaking
- 🧱 Block placement
- 🔄 Dynamic chunk mesh rebuilding
- ✂️ Near-plane triangle clipping
- 👁️ World-space face culling
- 📐 Perspective-correct STQ texturing
- 🖥️ Basic menu and HUD
- ⚡ 50 FPS / 50 VPS target on PAL

---

## 📸 Screenshots

> Screenshots and gameplay footage coming soon.

---

## 🎮 Controls

| Control | Action |
|---|---|
| Left Stick | Move |
| Right Stick | Look |
| ✕ Cross | Jump |
| □ Square | Sprint |
| R2 | Mine block |
| L2 | Place block |
| ○ Circle | Back / Menu |

---

## 🧱 Rendering

MikosCraft contains its own voxel rendering pipeline built around the PlayStation 2 Graphics Synthesizer.

```text
                 WORLD
                   │
                   ▼
                CHUNKS
                   │
                   ▼
          EXPOSED FACE MESH
                   │
                   ▼
         WORLD-SPACE CULLING
                   │
                   ▼
          CAMERA TRANSFORM
                   │
                   ▼
         NEAR-PLANE CLIPPING
                   │
                   ▼
       PERSPECTIVE PROJECTION
                   │
                   ▼
     PERSPECTIVE-CORRECT STQ
                   │
                   ▼
          PLAYSTATION 2 GS
```

The renderer currently performs CPU-side geometry processing on the **Emotion Engine** and submits textured triangles through **gsKit**.

Several optimizations are already implemented:

- cached chunk meshes
- exposed-face meshing
- world-space face culling
- per-frame camera transform caching
- near-plane clipping
- perspective-correct texture mapping

Future versions are planned to experiment with larger geometry batches, DMA and VU1-assisted rendering.

---

## 🌍 World

The world is divided into chunks.

Current chunk dimensions:

```text
8 × 32 × 8 blocks
```

The current development build keeps a small group of chunks resident in memory while the world/chunk manager is being developed.

Terrain generation is deterministic and based on a world seed.

Example:

```c
world_generate(&game->world, 2137);
```

### Current terrain

The generator currently makes use of:

- Grass
- Dirt
- Stone
- Bedrock

The block registry already supports additional block types including:

- Cobblestone
- Sand
- Gravel
- Oak Planks
- Oak Logs
- Oak Leaves
- Glass

---

## ⛏️ Block Interaction

Blocks are selected using a first-person raycast originating from the player's camera.

### Mining

Mining is performed by holding **R2**.

Blocks have individual hardness values, allowing different block types to require different amounts of time to break.

This provides the foundation for a future tool system:

```text
Hand
  │
  ▼
Wooden Tools
  │
  ▼
Stone Tools
  │
  ▼
Iron Tools
  │
  ▼
Diamond Tools
```

Mining progress is cancelled when the player releases R2 or starts targeting another block.

### Placement

Blocks can be placed using **L2**.

The current development implementation places a test block while the inventory and hotbar systems are still under development.

Placement includes collision checking to prevent the player from placing a block inside their own collision box.

---

## 🎯 Raycasting

Block interaction uses a ray cast from the player's eye position in the direction of the camera.

The ray determines:

- the targeted block
- the targeted block coordinates
- the block type
- the adjacent empty voxel used for placement

This system is used by both mining and block placement.

---

## ✂️ Near-Plane Clipping

MikosCraft implements near-plane triangle clipping before perspective projection.

This prevents geometry close to the camera from disappearing when only part of a triangle crosses the near plane.

Instead of discarding an entire triangle:

```text
             NEAR PLANE
                 │
Camera           │
  O              │
                 │────── B
           A ────┼██████│
                 │██████│
           D ────┼██████│
                 │────── C
```

the geometry is clipped against the near plane:

```text
             NEAR PLANE
                 │
                 │────── B
                 │██████│
                 │██████│
                 │██████│
                 │────── C
```

Texture coordinates are interpolated during clipping before perspective-correct STQ coordinates are generated.

This allows the player to stand extremely close to blocks without nearby geometry disappearing.

---

## 📐 Perspective-Correct Texturing

The PlayStation 2 Graphics Synthesizer supports perspective-correct texture mapping using **STQ coordinates**.

MikosCraft calculates:

```text
Q = 1 / Z

S = U × Q
T = V × Q
```

The GS then interpolates `S`, `T`, and `Q` across the triangle.

This prevents the severe texture distortion that would otherwise occur when rendering voxel surfaces at steep viewing angles.

---

## ⚡ Chunk Meshing

MikosCraft does not render every face of every block.

Instead, chunk meshes contain only block faces that are exposed to air.

For example:

```text
████████████████
████████████████
████████████████
```

Internal faces between adjacent solid blocks are never added to the render mesh.

When a block is changed:

```text
world_set_block(...)
        │
        ▼
chunk->mesh_dirty = 1
        │
        ▼
chunk mesh rebuilt
```

Chunks adjacent to modified chunk boundaries can also be marked dirty so their shared geometry is updated correctly.

This dramatically reduces the amount of geometry submitted to the renderer.

---

## 🚀 Performance

The current development target is:

```text
50 FPS
50 VPS
100% game speed
```

on PAL PlayStation 2 hardware / equivalent emulation.

Performance work currently includes:

- chunk mesh caching
- exposed-face generation
- face visibility rejection
- camera transform caching
- avoiding repeated trigonometric calculations per vertex
- near-plane clipping
- reduced GS submissions where possible

More aggressive PS2-specific optimization is planned.

---

## 🗺️ Roadmap

### Gameplay

- [x] Player movement
- [x] Analog camera
- [x] Gravity
- [x] Jumping
- [x] Sprinting
- [x] Player collision
- [x] Block raycasting
- [x] Hold-to-mine block breaking
- [x] Block placement
- [ ] Block breaking crack animation
- [ ] Item drops
- [ ] 9-slot hotbar
- [ ] Inventory
- [ ] Tools
- [ ] Tool durability
- [ ] Crafting
- [ ] Health
- [ ] Hunger
- [ ] Survival mechanics

### World

- [x] Chunk storage
- [x] Seed-based generation
- [x] Dynamic mesh rebuilding
- [ ] Dynamic chunk loading
- [ ] Chunk unloading
- [ ] Larger worlds
- [ ] Improved terrain noise
- [ ] Biomes
- [ ] Trees
- [ ] Caves
- [ ] Ores
- [ ] Water
- [ ] Lighting
- [ ] Save / load

### Rendering

- [x] Textured voxel rendering
- [x] Texture atlas
- [x] Perspective-correct STQ
- [x] Exposed-face meshing
- [x] World-space face culling
- [x] Camera transform cache
- [x] Near-plane clipping
- [ ] Chunk frustum culling
- [ ] Geometry batching
- [ ] Reduced draw submissions
- [ ] DMA optimization
- [ ] VU1-assisted rendering
- [ ] Transparent block rendering
- [ ] Render-distance management
- [ ] Fog

### UI

- [x] Main menu
- [x] Crosshair
- [ ] Minecraft-style hotbar
- [ ] Inventory screen
- [ ] Crafting screen
- [ ] Pause menu
- [ ] Settings
- [ ] Debug overlay improvements

### Audio

- [ ] Sound effects
- [ ] Block sounds
- [ ] Music playback
- [ ] Random music scheduling
- [ ] Ambient audio

---

## 🔧 Building

MikosCraft requires a working PlayStation 2 homebrew development environment.

### Dependencies

- PS2DEV
- PS2SDK
- gsKit
- EE GCC toolchain

Example environment:

```bash
export PS2DEV=/opt/ps2dev
export PS2SDK=$PS2DEV/ps2sdk
export GSKIT=$PS2DEV/gsKit

export PATH=$PATH:$PS2DEV/bin
export PATH=$PATH:$PS2DEV/ee/bin
```

Build the project with:

```bash
make -j4
```

The resulting executable is:

```text
MIKOSCRAFT.ELF
```

The ELF can then be launched through a PlayStation 2 emulator or transferred to compatible real PS2 homebrew hardware.

---

## 📁 Project Structure

```text
MikosCraft/
│
├── include/
│   ├── block.h
│   ├── chunk_mesh.h
│   ├── game.h
│   ├── interaction.h
│   ├── mikos_input.h
│   ├── player.h
│   ├── renderer.h
│   ├── voxel_renderer.h
│   └── world.h
│
├── src/
│   ├── main.c
│   ├── game.c
│   ├── input.c
│   ├── interaction.c
│   ├── player.c
│   ├── renderer.c
│   │
│   └── world/
│       ├── block.c
│       ├── chunk_mesh.c
│       ├── voxel_renderer.c
│       └── world.c
│
├── assets/
├── generated/
├── tools/
│
├── Makefile
└── README.md
```

---

## 🧠 PlayStation 2

MikosCraft is also an experiment in building a modern-style voxel sandbox within the constraints of sixth-generation console hardware.

The PlayStation 2 provides roughly:

```text
┌─────────────────────────────────┐
│         PLAYSTATION 2           │
├─────────────────────────────────┤
│ Emotion Engine                  │
│ ~294 MHz                        │
│                                 │
│ Main RAM                        │
│ 32 MB RDRAM                     │
│                                 │
│ Graphics Synthesizer            │
│ 4 MB embedded DRAM              │
│                                 │
│ Vector Units                    │
│ VU0 + VU1                       │
└─────────────────────────────────┘
```

These constraints mean MikosCraft cannot simply approach voxel rendering like a modern PC game.

The project therefore focuses heavily on:

- compact world representation
- visibility rejection
- cached geometry
- minimal runtime allocation
- small chunk sizes
- reduced CPU overhead
- efficient CPU → GS communication
- PS2-specific optimization

---

## 🔬 Renderer Goals

The current renderer is intentionally being developed in stages.

### Current

```text
CPU / Emotion Engine

ChunkMesh
    ↓
Face Culling
    ↓
Camera Transform
    ↓
Near Clipping
    ↓
Projection
    ↓
STQ Generation
    ↓
gsKit
    ↓
Graphics Synthesizer
```

### Future

The long-term renderer design may move toward:

```text
Chunk
  ↓
Chunk Frustum Culling
  ↓
Optimized Mesh
  ↓
Geometry Batches
  ↓
DMA
  ↓
VU1
  ↓
Graphics Synthesizer
```

The goal is to increase render distance while maintaining the target frame rate.

---

## 💡 Inspiration

MikosCraft is inspired by **Minecraft** and by existing PlayStation 2 homebrew projects demonstrating what the hardware can achieve.

### TyraCraft / Tyra Engine

TyraCraft and Tyra Engine are used as **technical references only**.

MikosCraft:

- is not a TyraCraft fork
- does not use Tyra Engine as a dependency
- has its own world implementation
- has its own player controller
- has its own chunk system
- has its own interaction system
- has its own rendering code

Ideas from other voxel engines may be studied to better understand efficient approaches to rendering and game architecture on constrained hardware.

---

## 🎨 Assets

Minecraft assets are **not distributed with this repository**.

During private development, MikosCraft can use textures extracted from a legally obtained Minecraft installation.

Users must provide any such assets themselves.

The MikosCraft source repository intentionally excludes those assets.

---

## ⚠️ Disclaimer

MikosCraft is an independent, non-commercial hobby project.

It is not affiliated with, endorsed by, sponsored by, or associated with:

- Mojang Studios
- Microsoft
- Sony Interactive Entertainment
- Tyra Engine
- TyraCraft

**Minecraft** is a trademark of Microsoft Corporation.

**PlayStation** and **PlayStation 2** are trademarks of Sony Interactive Entertainment.

---

## 🛠️ Development Status

```text
MIKOSCRAFT
PLAYSTATION 2 EDITION

          PRE-ALPHA

World generation       [██████████] WORKING
Chunk system           [██████████] WORKING
Voxel renderer         [██████████] WORKING
Player movement        [██████████] WORKING
Collision              [██████████] WORKING
Mining                 [██████████] WORKING
Block placement        [██████████] WORKING
Inventory              [░░░░░░░░░░] TODO
Crafting               [░░░░░░░░░░] TODO
Survival               [░░░░░░░░░░] TODO
Mobs                    [░░░░░░░░░░] TODO
```

---

<div align="center">

## MIKOSCRAFT

**PLAYSTATION 2 EDITION**

*Building a voxel world on hardware from 2000.*

### because why the hell not.

</div>
