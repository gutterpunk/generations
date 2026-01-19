/**
 * Map Module
 * Handles level data and rendering
 **/
#include <genesis.h>
#include "map.h"
#include "game.h"

void mapInit(u8 width, u8 height)
{
    currentState.width = width;
    currentState.height = height;
    currentState.moveCount = 0;
    currentState.playerX = width / 2;
    currentState.playerY = height / 2;
    historyIndex = 0;
    
    memset(currentState.grid, TILE_EMPTY, MAX_GRID_SIZE * MAX_GRID_SIZE);
    memset(currentState.state, STATE_STATIONARY, MAX_GRID_SIZE * MAX_GRID_SIZE);
    
    u16 emptyTile = TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, 0);
    VDP_fillTileMapRect(BG_A, emptyTile, 0, 0, 64, 32);
    
    for (u8 x = 0; x < width; x++) {
        for (u8 y = 0; y < height; y++) {
            currentState.grid[x][y] = TILE_DIRT;
        }
    }
    
    if (width >= 5 && height >= 5) {
        currentState.grid[3][2] = TILE_BOULDER;
        currentState.grid[5][2] = TILE_BOULDER;
        currentState.grid[4][3] = TILE_DIAMOND;
        currentState.grid[6][3] = TILE_DIAMOND;
    }
    
    currentState.grid[currentState.playerX][currentState.playerY] = TILE_PLAYER;
    gameSaveState();
}

void mapDrawPlayfield()
{
    u8 offsetX = (64 - (currentState.width * 2)) / 2;
    u8 offsetY = (32 - (currentState.height * 2)) / 2;
    
    for (u8 x = 0; x < currentState.width; x++) {
        for (u8 y = 0; y < currentState.height; y++) {
            mapDrawTileAt(offsetX + (x * 2), offsetY + (y * 2), currentState.grid[x][y]);
        }
    }
}

void mapDrawTileAt(u8 screenX, u8 screenY, u8 tileType)
{
    u16 baseTile = tileType * 4;
    
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, baseTile + 0), screenX, screenY);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, baseTile + 1), screenX + 1, screenY);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, baseTile + 2), screenX, screenY + 1);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, baseTile + 3), screenX + 1, screenY + 1);
}
