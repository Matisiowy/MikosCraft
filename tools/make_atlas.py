from PIL import Image
import random

SIZE = 256
TILE = 16

img = Image.new("RGBA", (SIZE, SIZE), (0,0,0,0))

def px(tile, x, y, c):
    tx = (tile % 16) * TILE
    ty = (tile // 16) * TILE
    img.putpixel((tx+x, ty+y), (*c,255))

def fill(tile, color):
    for y in range(16):
        for x in range(16):
            px(tile,x,y,color)

random.seed(2137)

# ------------------------------------------------------------
# TILE 0 - GRASS TOP
# ------------------------------------------------------------

fill(0, (91, 158, 63))

grass_colors = [
    (79,143,55),
    (104,170,68),
    (70,132,48),
    (119,180,75),
    (86,151,57)
]

for i in range(75):
    x=random.randrange(16)
    y=random.randrange(16)
    px(0,x,y,random.choice(grass_colors))

# ------------------------------------------------------------
# TILE 1 - GRASS SIDE
# ------------------------------------------------------------

fill(1, (121, 86, 54))

dirt_colors = [
    (110,76,47),
    (128,91,56),
    (101,69,43),
    (139,99,61)
]

for y in range(16):
    for x in range(16):
        if random.random() < .28:
            px(1,x,y,random.choice(dirt_colors))

# trawa u góry
for x in range(16):
    depth=random.choice([3,3,4,4,5])

    for y in range(depth):
        px(
            1,x,y,
            random.choice(grass_colors)
        )

# zwisające źdźbła
for x in range(16):
    if random.random() < .35:
        depth=random.randrange(4,7)
        px(1,x,depth,random.choice(grass_colors))

# ------------------------------------------------------------
# TILE 2 - DIRT
# ------------------------------------------------------------

fill(2,(121,86,54))

for i in range(90):
    x=random.randrange(16)
    y=random.randrange(16)
    px(2,x,y,random.choice(dirt_colors))

# ------------------------------------------------------------
# TILE 3 - STONE
# ------------------------------------------------------------

fill(3,(125,125,125))

stone_colors=[
    (105,105,105),
    (115,115,115),
    (135,135,135),
    (145,145,145),
    (96,96,96)
]

for i in range(105):
    x=random.randrange(16)
    y=random.randrange(16)
    px(3,x,y,random.choice(stone_colors))

# małe skupiska
for i in range(20):
    x=random.randrange(1,15)
    y=random.randrange(1,15)
    c=random.choice(stone_colors)

    px(3,x,y,c)

    if random.random()<.5:
        px(3,x+1,y,c)

    if random.random()<.5:
        px(3,x,y+1,c)

# ------------------------------------------------------------
# TILE 4 - BEDROCK na przyszłość
# ------------------------------------------------------------

fill(4,(70,70,70))

bedrock=[
    (45,45,45),
    (60,60,60),
    (85,85,85),
    (105,105,105)
]

for i in range(130):
    px(
        4,
        random.randrange(16),
        random.randrange(16),
        random.choice(bedrock)
    )

img.save("assets/textures/terrain.png")

print("Created assets/textures/terrain.png")
