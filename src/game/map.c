/**
 * Map Module
 * Handles level data and rendering
 **/
#include <genesis.h>
#include "map.h"
#include "engine.h"
#include "game.h"
#include "resources.h"
#include "tiles.h"
#include "gdr_reader.h"

u32 frameCounter;

u16 vramState[MAX_GRID_SIZE*2][MAX_GRID_SIZE*2];

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

void setBothMapTile(u8 x, u8 y, u8 tileIndex) {
    const TileMapping* mapping = getTileMapping(tileIndex);
    u16 tl = mapping->topLeft;
    u16 tr = mapping->topRight;
    u16 bl = mapping->bottomLeft;
    u16 br = mapping->bottomRight;
    
    gameState.betweenFramesGrid[x][y] = tl;
    gameState.betweenFramesGrid[x + 1][y] = tr;
    gameState.betweenFramesGrid[x][y + 1] = bl;
    gameState.betweenFramesGrid[x + 1][y + 1] = br;

    gameState.visualGrid[x][y] = tl;
    gameState.visualGrid[x + 1][y] = tr;
    gameState.visualGrid[x][y + 1] = bl;
    gameState.visualGrid[x + 1][y + 1] = br;
}

/* GDR character to tile type mapping */
static u8 mapGDRCharToTile(char c) {
    switch(c) {
        case '.': return TILE_EMPTY;
        case '#': return TILE_WALL;
        case '*': return TILE_DIRT;
        case '0': return TILE_BOULDER;
        case 'd': return TILE_DIAMOND;
        case '%': return TILE_STEEL;
        case 'X': return TILE_EXIT;
        case '<': return TILE_FIREFLY;
        default: return TILE_EMPTY;
    }
}

void mapInit(u16 boardIndex)
{
    extern const u8 gdr_data[];
    GDRBoard board;
    GDRSolution solution;
    
    /* Load board from GDR data - crash if fails */
    if (!GDR_LoadBoardData(gdr_data, boardIndex, &board, &solution)) {
        fatalError("Failed to load board data");
    }
    
    gameState.gridWidth = board.width;
    gameState.gridHeight = board.height;
    gameState.playerX = board.startX;
    gameState.playerY = board.startY;
    gameState.moveCount = 0;
    gameState.historyIndex = 0;
    
    memset(gameState.visualGrid, TILE_EMPTY, MAX_GRID_SIZE * 2 * MAX_GRID_SIZE * 2);
    
    u16 emptyTile = TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, 0);
    VDP_fillTileMapRect(BG_A, emptyTile, 0, 0, 64, 32);
    
    for (u8 y = 0; y < gameState.gridHeight; y++) {
        for (u8 x = 0; x < gameState.gridWidth; x++) {
            u16 dataIdx = y * board.width + x;
            char c = (dataIdx < GDR_MAX_DATA) ? board.data[dataIdx] : ' ';
            u8 tileType = mapGDRCharToTile(c);
            
            u8 gx = x * 2;
            u8 gy = y * 2;
            
            /* Skip player tile  */
            if (c != '@') {
                setVisualMapTile(gx, gy, tileType);
                gameState.objectGrid[x][y].object = tileType;
                SET_STATE(gameState.objectGrid[x][y], STATE_STATIONARY);
            } else {
                setVisualMapTile(gx, gy, TILE_DIRT);
                gameState.objectGrid[x][y].object = TILE_DIRT;
                SET_STATE(gameState.objectGrid[x][y], STATE_STATIONARY);
            }
        }
    }
    
    /* Place player */
    setVisualMapTile(gameState.playerX * 2, gameState.playerY * 2, TILE_PLAYER);
    gameState.objectGrid[gameState.playerX][gameState.playerY].object = TILE_PLAYER;
    SET_STATE(gameState.objectGrid[gameState.playerX][gameState.playerY], STATE_STATIONARY);

    /* Place exit */
    setVisualMapTile(board.exitX * 2, board.exitY * 2, TILE_EXIT);
    gameState.objectGrid[board.exitX][board.exitY].object = TILE_EXIT;
    SET_STATE(gameState.objectGrid[board.exitX][board.exitY], STATE_STATIONARY);

    memcpy(&gameState.original, &gameState.objectGrid, sizeof(gameState.objectGrid));
    gameSaveState();
}

void mapDrawPlayfield(bool inBetweenFrames)
{
    u8 (*sourceGrid)[MAX_GRID_SIZE * 2] = inBetweenFrames 
        ? gameState.betweenFramesGrid 
        : gameState.visualGrid;
    frameCounter++;
    u8 offsetX = (64 - (gameState.gridWidth * 2)) / 2;
    u8 offsetY = (32 - (gameState.gridHeight * 2)) / 2;
    for (u8 x = 0; x < gameState.gridWidth * 2; x++) {
        for (u8 y = 0; y < gameState.gridHeight * 2; y++) {
            u16 tileIndex = sourceGrid[x][y] + TILE_USER_INDEX; 
            if (vramState[x][y] != tileIndex) {

                vramState[x][y] = tileIndex;
                VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, tileIndex), offsetX + x, offsetY + y);
            }
        }
    }
}



