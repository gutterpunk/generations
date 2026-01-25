/**
 * Map Module
 * Handles level data and rendering
 **/
#include <genesis.h>
#include "map.h"
#include "game.h"
#include "resources.h"

u32 frameCounter;

void mapInit(u8 width, u8 height)
{
    gameState.width = width;
    gameState.height = height;
    gameState.moveCount = 0;
    gameState.playerX = width / 2;
    gameState.playerY = height / 2;
    gameState.historyIndex = 0;
    
    memset(gameState.grid, TILE_EMPTY, MAX_GRID_SIZE * MAX_GRID_SIZE);
    memset(gameState.state, STATE_STATIONARY, MAX_GRID_SIZE * MAX_GRID_SIZE);
    
    u16 emptyTile = TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, 0);
    VDP_fillTileMapRect(BG_A, emptyTile, 0, 0, 64, 32);
    
    for (u8 x = 0; x < width; x++) {
        for (u8 y = 0; y < height; y++) {
            gameState.grid[x][y] = TILE_DIRT;
        }
    }
    
    if (width >= 5 && height >= 5) {
        gameState.grid[3][2] = TILE_BOULDER;
        gameState.grid[5][2] = TILE_BOULDER;
        gameState.grid[4][3] = TILE_DIAMOND;
        gameState.grid[6][3] = TILE_DIAMOND;
    }
    
    gameState.grid[gameState.playerX][gameState.playerY] = TILE_PLAYER;
    memcpy(&gameState.original, &gameState.grid, sizeof(gameState.original));
    gameSaveState();
}

void mapDrawPlayfield()
{
    frameCounter++;
    u8 offsetX = (64 - (gameState.width * 2)) / 2;
    u8 offsetY = (32 - (gameState.height * 2)) / 2;
    for (u8 x = 0; x < gameState.width; x++) {
        for (u8 y = 0; y < gameState.height; y++) {
            mapDrawTileAt(offsetX + (x * 2), offsetY + (y * 2), gameState.grid[x][y]);
        }
    }
}

void mapDrawTileAt(u8 screenX, u8 screenY, u8 tileType)
{
    if (!needsRedraw && tileType != TILE_DIAMOND) return;
    u16 tilemapRow = bg_tilemap.tilemap->w;
    u16 col = (tileType * 2) % tilemapRow;
    u16 row = (tileType * 2) / tilemapRow;
    u16 baseTile = (row * tilemapRow * 2) + col;
    baseTile += TILE_USER_INDEX;
    if (tileType == TILE_DIAMOND) {
        baseTile += ((frameCounter / 4) % 4) * 2; 
    }
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, baseTile + 0), screenX, screenY);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, baseTile + 1), screenX + 1, screenY);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, baseTile + tilemapRow), screenX, screenY + 1);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, baseTile + tilemapRow + 1), screenX + 1, screenY + 1);
}
