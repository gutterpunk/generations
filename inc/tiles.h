#ifndef TILES_H
#define TILES_H

#include <genesis.h>
#include "game.h"

#define TILE_STRIDE 32

#define TILE_EMPTY 2
#define TILE_STEEL 1
#define TILE_WALL 0
#define TILE_DIRT 4
#define TILE_BOULDER 3
#define TILE_EXIT 18
#define TILE_PLAYER 16
#define TILE_DIAMOND 32

#define TILE_FIREFLY 5  // amoeba
#define TILE_BUTTERFLY 43 // steel amoeba

#define TILE_MAX 44

typedef struct {
    u16 topLeft;
    u16 topRight;
    u16 bottomLeft;
    u16 bottomRight;
    u8 frames;
} TileMapping;

extern const TileMapping tileMappings[];
extern const u8 tileMappingCount;
extern const TileMapping* getTileMapping(u8 tileIndex);

#endif