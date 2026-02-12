#ifndef ENGINE_H
#define ENGINE_H

#include <genesis.h>

#define TILE_PIXEL_SIZE 16
#define TILES_PER_BLOCK 2

extern char* fatalErrorString;
extern u16 basePalette[16];
extern u16 darkenedPalette[16];

void engineInit();
void engineRender();
void engineUpdateCamera();
//void engineDrawUI();
void fatalError(const char* message);
void refreshBackground();
#endif
