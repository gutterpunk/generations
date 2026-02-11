/**
 * Engine Module
 * Handles rendering, camera, and display
 **/
#include <genesis.h>
#include "resources.h"
#include "engine.h"
#include "game.h"
#include "map.h"
#include "tiles.h"

char* fatalErrorString = NULL;
u16 basePalette[16];
u16 darkenedPalette[16];

void engineInit()
{
    VDP_setScreenWidth320();
    VDP_setPlaneSize(64, 32, TRUE);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    for (u16 i = 0; i < 16; i++) {
        u16 color = bg_tilemap.palette->data[i];
        basePalette[i] = color;

        u16 r = ((color >> 1) & 0x7) >> 1;
        u16 g = ((color >> 5) & 0x7) >> 1;
        u16 b = ((color >> 9) & 0x7) >> 1;
        darkenedPalette[i] = (b << 9) | (g << 5) | (r << 1);
    }

    PAL_setPalette(PAL0, basePalette, DMA);
    PAL_setPalette(PAL1, basePalette, DMA);

    VDP_loadTileSet(bg_tilemap.tileset, TILE_USER_INDEX, DMA);
    PAL_setColor(63, RGB24_TO_VDPCOLOR(0xFFFFFF));
    JOY_init();
}

void fatalError(const char* message) {
    VDP_clearPlane(BG_A, TRUE);
    VDP_drawText("FATAL ERROR:", 10, 10);
    VDP_drawText(message, 10, 12);
    if (fatalErrorString) {
        VDP_drawText(fatalErrorString, 10, 13);
    }
    VDP_drawText("System Halted", 10, 15);
    
    while(1) {
        SYS_doVBlankProcess();
    }
}
void engineRender()
{
    switch (redrawStage)
    {
        case REDRAW_STAGE_BETWEEN:
            mapDrawPlayfield(TRUE);
            redrawStage = REDRAW_STAGE_VISUAL;
            break;
        case REDRAW_STAGE_VISUAL:
            mapDrawPlayfield(FALSE);
            redrawStage = REDRAW_STAGE_NONE;
            break;
    }

    
   
    engineUpdateCamera();
    //engineDrawDebugOverlay();
}

void engineUpdateCamera()
{
    u8 mapOffsetTilesX = (64 - (gameState.gridWidth * 2)) / 2;
    u8 mapOffsetTilesY = (32 - (gameState.gridHeight * 2)) / 2;
    s16 mapOffsetX = mapOffsetTilesX * 8;
    s16 mapOffsetY = mapOffsetTilesY * 8;
    
    s16 mapWidth = gameState.gridWidth * 16;
    s16 mapHeight = gameState.gridHeight * 16;
    
    s16 cameraX, cameraY;
    
    if (mapWidth <= 320 && mapHeight <= 224) {
        cameraX = mapOffsetX - ((320 - mapWidth) / 2);
        cameraY = mapOffsetY - ((224 - mapHeight) / 2);
    } else {
        // Player position is in logical coordinates, convert to pixels
        s16 playerPixelX = (gameState.playerX * 16) + mapOffsetX;
        s16 playerPixelY = (gameState.playerY * 16) + mapOffsetY;
        
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

