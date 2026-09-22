from PIL import Image
from pathlib import Path

ROOT = Path("/opt/mikoscraft")
SRC  = ROOT / "assets/minecraft-1.8.9/textures"
OUT  = ROOT / "assets/textures/terrain.png"

TILE = 16
ATLAS_TILES = 16
SIZE = TILE * ATLAS_TILES

textures = [
    "grass_top.png",       # 0
    "grass_side.png",      # 1
    "dirt.png",            # 2
    "stone.png",           # 3
    "bedrock.png",         # 4
    "cobblestone.png",     # 5
    "sand.png",            # 6
    "gravel.png",          # 7
    "planks_oak.png",      # 8
    "log_oak.png",         # 9
    "log_oak_top.png",     # 10
    "leaves_oak.png",      # 11
    "glass.png",           # 12
    "coal_ore.png",        # 13
    "iron_ore.png",        # 14
    "gold_ore.png",        # 15
    "diamond_ore.png",     # 16
    "redstone_ore.png",    # 17
    "lapis_ore.png",       # 18
    "emerald_ore.png",     # 19
]

atlas = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))

for index, filename in enumerate(textures):
    path = SRC / filename

    if not path.exists():
        raise FileNotFoundError(path)

    img = Image.open(path).convert("RGBA")

    if img.size != (16, 16):
        print(f"[RESIZE] {filename}: {img.size} -> 16x16")
        img = img.resize((16, 16), Image.Resampling.NEAREST)

    tx = index % ATLAS_TILES
    ty = index // ATLAS_TILES

    x = tx * TILE
    y = ty * TILE

    atlas.paste(img, (x, y), img)

    print(
        f"[{index:02d}] "
        f"{filename:<20} "
        f"tile=({tx},{ty}) "
        f"pixel=({x},{y})"
    )

OUT.parent.mkdir(parents=True, exist_ok=True)
atlas.save(OUT)

print()
print(f"Atlas written: {OUT}")
print(f"Size: {atlas.size[0]}x{atlas.size[1]}")
