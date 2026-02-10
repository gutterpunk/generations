#ifndef ENGINE_H
#define ENGINE_H

#include <genesis.h>

#define TILE_PIXEL_SIZE 16
#define TILES_PER_BLOCK 2

extern char* fatalErrorString;

void engineInit();
void engineRender();
void engineUpdateCamera();
//void engineDrawUI();
void engineDrawDebugOverlay();
void fatalError(const char* message);

#endif
