from PIL import Image

src = "assets/textures/terrain.png"
dst = "generated/terrain_atlas.c"

im = Image.open(src).convert("RGBA")

if im.size != (256,256):
    raise SystemExit("terrain.png must be 256x256")

pixels = list(im.getdata())

with open(dst, "w") as f:
    f.write('#include <tamtypes.h>\n\n')
    f.write('__attribute__((aligned(128)))\n')
    f.write('u32 terrain_atlas_pixels[256 * 256] = {\n')

    for i,(r,g,b,a) in enumerate(pixels):

        # little-endian u32 -> memory bytes R,G,B,A
        value = (
            r |
            (g << 8) |
            (b << 16) |
            (a << 24)
        )

        f.write('0x%08X' % value)

        if i != len(pixels)-1:
            f.write(',')

        if i % 8 == 7:
            f.write('\n')
        else:
            f.write(' ')

    f.write('};\n')

print("generated:", dst)
