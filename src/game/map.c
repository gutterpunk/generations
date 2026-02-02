/**
 * Map Module
 * Handles level data and rendering
 **/
#include <genesis.h>
#include "map.h"
#include "game.h"
#include "resources.h"
#include "tiles.h"

u32 frameCounter;

void setVisualMapTile(u8 x, u8 y, u8 tileIndex) {
    const TileMapping* mapping = getTileMapping(tileIndex);
    gameState.visualGrid[x][y] = mapping->topLeft;
    gameState.visualGrid[x + 1][y] = mapping->topRight;
    gameState.visualGrid[x][y + 1] = mapping->bottomLeft;
    gameState.visualGrid[x + 1][y + 1] = mapping->bottomRight;
}

void setBetweenFramesMapTile(u8 x, u8 y, u8 tileIndex) {
    const TileMapping* mapping = getTileMapping(tileIndex);
    gameState.betweenFramesGrid[x][y] = mapping->topLeft;
    gameState.betweenFramesGrid[x + 1][y] = mapping->topRight;
    gameState.betweenFramesGrid[x][y + 1] = mapping->bottomLeft;
    gameState.betweenFramesGrid[x + 1][y + 1] = mapping->bottomRight;
}

void mapInit(u8 width, u8 height)
{
    gameState.gridWidth = width;
    gameState.gridHeight = height;
    gameState.moveCount = 0;
    gameState.playerX = width / 2;
    gameState.playerY = height / 2;
    gameState.historyIndex = 0;
    
    memset(gameState.visualGrid, TILE_EMPTY, MAX_GRID_SIZE * 2 * MAX_GRID_SIZE * 2);
    
    u16 emptyTile = TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, 0);
    VDP_fillTileMapRect(BG_A, emptyTile, 0, 0, 64, 32);
    
    // Fill  grid with tiles
    for (u8 x = 0; x < width; x++) {
        for (u8 y = 0; y < height; y++) {
            u8 gx = x * 2;
            u8 gy = y * 2;
            
            setVisualMapTile(gx, gy, TILE_DIRT);
            gameState.objectGrid[x][y].object = TILE_DIRT;
            gameState.objectGrid[x][y].state = STATE_STATIONARY;
        }
    }
    
    // Place objects
    if (width >= 5 && height >= 5) {
        u8 bx1 = 3, by1 = 2;
        setVisualMapTile(bx1 * 2, by1 * 2, TILE_BOULDER);
        gameState.objectGrid[bx1][by1].object = TILE_BOULDER;
        gameState.objectGrid[bx1][by1].state = STATE_STATIONARY;

        bx1 = 5, by1 = 2;
        setVisualMapTile(bx1 * 2, by1 * 2, TILE_BOULDER);
        gameState.objectGrid[bx1][by1].object = TILE_BOULDER;
        gameState.objectGrid[bx1][by1].state = STATE_STATIONARY;
        
        bx1 = 4, by1 = 3;
        setVisualMapTile(bx1 * 2, by1 * 2, TILE_DIAMOND);
        gameState.objectGrid[bx1][by1].object = TILE_DIAMOND;
        gameState.objectGrid[bx1][by1].state = STATE_STATIONARY;
        
        bx1 = 6, by1 = 3;
        setVisualMapTile(bx1 * 2, by1 * 2, TILE_DIAMOND);
        gameState.objectGrid[bx1][by1].object = TILE_DIAMOND;
        gameState.objectGrid[bx1][by1].state = STATE_STATIONARY;
    }
    
    // Place player 
    setVisualMapTile(gameState.playerX * 2, gameState.playerY * 2, TILE_PLAYER);

    gameState.objectGrid[gameState.playerX][gameState.playerY].object = TILE_PLAYER;
    gameState.objectGrid[gameState.playerX][gameState.playerY].state = STATE_STATIONARY;

    // Save original state
    memcpy(&gameState.original, &gameState.objectGrid, sizeof(gameState.objectGrid));
    gameSaveState();
}

void mapDrawPlayfield()
{
    frameCounter++;
    u8 offsetX = (64 - (gameState.gridWidth * 2)) / 2;
    u8 offsetY = (32 - (gameState.gridHeight * 2)) / 2;
    for (u8 x = 0; x < gameState.gridWidth * 2; x++) {
        for (u8 y = 0; y < gameState.gridHeight * 2; y++) {
            u16 tileIndex = gameState.visualGrid[x][y] + TILE_USER_INDEX;
            VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, tileIndex), offsetX + x, offsetY + y);
        }
    }
}

