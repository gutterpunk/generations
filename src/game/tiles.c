#include "tiles.h"


static const TileMapping tileLookup[TILE_MAX] = {
    [TILE_WALL] = {0, 1, TILE_STRIDE, TILE_STRIDE + 1, 1},
    [TILE_EMPTY] = { 4, 5, TILE_STRIDE + 4, TILE_STRIDE + 5, 1},
    [TILE_BOULDER] = {6, 7, TILE_STRIDE + 6, TILE_STRIDE + 7, 1},
    [TILE_DIRT] = {8, 9, TILE_STRIDE + 8, TILE_STRIDE + 9, 1},
    [TILE_PLAYER] = {64, 65, TILE_STRIDE + 64, TILE_STRIDE + 65, 1},
    [TILE_DIAMOND] = {128, 129, TILE_STRIDE + 128, TILE_STRIDE + 129, 4},
};

const u8 tileMappingCount = sizeof(tileLookup) / sizeof(TileMapping);

const TileMapping* getTileMapping(u8 tileIndex) {
    if (tileIndex < TILE_MAX) {
        return &tileLookup[tileIndex];
    }
    return &tileLookup[TILE_EMPTY]; // fallback... should be steel wall
}

TileMapping getTileFrame(u8 tileIndex) {
    TileMapping mapping = *getTileMapping(tileIndex);
    if (mapping.frames > 1) {
        u8 frame = frameCounter % mapping.frames;
        u16 offset = frame * 2;
        mapping.topLeft += offset;
        mapping.topRight += offset;
        mapping.bottomLeft += offset;
        mapping.bottomRight += offset;
    }
    return mapping;
}