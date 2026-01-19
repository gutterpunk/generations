/**
 * Engine Module
 * Handles rendering, camera, and display
 **/
#include <genesis.h>
#include "resources.h"
#include "engine.h"
#include "game.h"
#include "map.h"

void engineInit()
{
    VDP_setScreenWidth320();
    VDP_setPlaneSize(64, 32, TRUE);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    PAL_setPalette(PAL0, bg_tilemap.palette->data, DMA);
    PAL_setColor(63, RGB24_TO_VDPCOLOR(0xFFFFFF));
    JOY_init();
    engineDrawUI();
}

void engineRender()
{
    mapDrawPlayfield();
    engineUpdateCamera();
    engineDrawDebugOverlay();
    
    char moveText[30];
    sprintf(moveText, "Moves: %d  ", currentState.moveCount);
    VDP_drawTextBG(BG_B, moveText, 1, 1);
    
    sprintf(moveText, "History: %d/%d  ", historyIndex, MAX_HISTORY);
    VDP_drawTextBG(BG_B, moveText, 1, 2);

    sprintf(moveText, "Physics: %d,%d  ", currentState.physicsX, currentState.physicsY);
    VDP_drawTextBG(BG_B, moveText, 1, 3);

    sprintf(moveText, "Player: %d,%d ", currentState.playerX, currentState.playerY);
    VDP_drawTextBG(BG_B, moveText, 1, 4);

    sprintf(moveText, "Waiting: %s  ", currentState.physicsWaitingForPlayer ? "Yes" : "No");
    VDP_drawTextBG(BG_B, moveText, 1, 5);
}

void engineUpdateCamera()
{
    u8 mapOffsetTilesX = (64 - (currentState.width * 2)) / 2;
    u8 mapOffsetTilesY = (32 - (currentState.height * 2)) / 2;
    s16 mapOffsetX = mapOffsetTilesX * 8;
    s16 mapOffsetY = mapOffsetTilesY * 8;
    
    s16 mapWidth = currentState.width * 16;
    s16 mapHeight = currentState.height * 16;
    
    s16 cameraX, cameraY;
    
    if (mapWidth <= 320 && mapHeight <= 224) {
        cameraX = mapOffsetX - ((320 - mapWidth) / 2);
        cameraY = mapOffsetY - ((224 - mapHeight) / 2);
    } else {
        s16 playerPixelX = (currentState.playerX * 16) + mapOffsetX;
        s16 playerPixelY = (currentState.playerY * 16) + mapOffsetY;
        
        cameraX = playerPixelX - 160 + 8;
        cameraY = playerPixelY - 112 + 8;
        
        s16 minScrollX = mapOffsetX;
        s16 minScrollY = mapOffsetY;
        s16 maxScrollX = mapOffsetX + mapWidth - 320;
        s16 maxScrollY = mapOffsetY + mapHeight - 224;
        
        if (cameraX < minScrollX) cameraX = minScrollX;
        if (cameraY < minScrollY) cameraY = minScrollY;
        if (cameraX > maxScrollX) cameraX = maxScrollX;
        if (cameraY > maxScrollY) cameraY = maxScrollY;
    }
    
    VDP_setHorizontalScroll(BG_A, -cameraX);
    VDP_setVerticalScroll(BG_A, cameraY);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
}

void engineDrawUI()
{
    VDP_drawTextBG(BG_B, "PERFECT DASH", 13, 0);
    VDP_drawTextBG(BG_B, "Arrow Keys: Move", 1, 26);
    VDP_drawTextBG(BG_B, "B: Rewind", 1, 27);
}

void engineDrawDebugOverlay()
{
    extern u8 physicsProcessed[MAX_GRID_SIZE][MAX_GRID_SIZE];
    
    u8 offsetX = (64 - (currentState.width * 2)) / 2;
    u8 offsetY = (32 - (currentState.height * 2)) / 2;
    offsetY -= 2;
    
    for (u8 x = 0; x < currentState.width; x++) {
        for (u8 y = 0; y < currentState.height; y++) {
            u8 tile = currentState.grid[x][y];
            u8 state = currentState.state[x][y];
            u8 processed = physicsProcessed[x][y];
            
            u8 screenX = offsetX + (x * 2);
            u8 screenY = offsetY + (y * 2);
            
            if (tile == TILE_BOULDER || tile == TILE_DIAMOND) {
                char stateChar[2];
                stateChar[0] = (state == STATE_FALLING) ? 'F' : 'S';
                stateChar[1] = '\0';
                VDP_drawTextBG(BG_B, stateChar, screenX, screenY);
                
                char procChar[2];
                procChar[0] = processed ? 'X' : 'O';
                procChar[1] = '\0';
                VDP_drawTextBG(BG_B, procChar, screenX + 1, screenY);
            }
            else                
                VDP_drawTextBG(BG_B, "  ", screenX, screenY);
        }
    }
}
