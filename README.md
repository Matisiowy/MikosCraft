cd /opt/mikoscraft || exit 1

cat > README.md <<'EOF'
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

## 🎮 Current state

MikosCraft is currently in **early pre-alpha development**.

The game can already generate a voxel world, render textured chunks, move a player through the world with collision and allow blocks to be mined and placed.

### Working

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
