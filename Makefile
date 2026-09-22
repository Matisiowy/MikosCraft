EE_BIN = MIKOSCRAFT.ELF

EE_OBJS = \
	build/main.o \
	build/game.o \
        build/player.o \
	build/interaction.o \
	build/input.o \
	build/renderer.o \
	build/font.o \
	build/texture.o \
	build/terrain_atlas.o \
	build/world.o \
	build/chunk_mesh.o \
	build/block.o \
	build/voxel_renderer.o

EE_INCS = \
	-I$(GSKIT)/include \
	-Iinclude

EE_LDFLAGS += \
	-L$(GSKIT)/lib \
	-L$(PS2SDK)/ports/lib

EE_LIBS = \
	-lgskit_toolkit \
	-lgskit \
	-ldmakit \
	-lpad \
	-lm

EE_CFLAGS = \
	-O2 \
	-G0 \
	-Wall

all: $(EE_BIN)


build/main.o: src/main.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@


build/player.o: src/player.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/game.o: src/game.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/input.o: src/input.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/renderer.o: src/renderer.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/font.o: src/font.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/texture.o: src/texture.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/terrain_atlas.o: generated/terrain_atlas.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/world.o: src/world/world.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@


build/chunk_mesh.o: src/world/chunk_mesh.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/voxel_renderer.o: src/world/voxel_renderer.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

clean:
	rm -rf build
	rm -f $(EE_BIN)
	rm -f MIKOSCRAFT-packed.ELF

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal


build/block.o: src/world/block.c
	@mkdir -p build
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

build/interaction.o: src/interaction.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

