/**
 * Player Module
 * Handles player input and movement
 **/
#include <genesis.h>
#include "engine.h"
#include "player.h"
#include "game.h"
#include "map.h"
#include "tiles.h"

extern u8 redrawStage;

void playerMoveOrAction(bool isPush, u8 newX, u8 newY, s8 dirX, s8 dirY) 
{
    if (!isPush) {
        setBothMapTile(gameState.playerX * 2, gameState.playerY * 2, TILE_EMPTY);
        gameState.objectGrid[gameState.playerX][gameState.playerY].object = TILE_EMPTY;
        SET_STATE(gameState.objectGrid[gameState.playerX][gameState.playerY], STATE_STATIONARY);
        
        setVisualMapTile(newX * 2, newY * 2, TILE_PLAYER);
        setBetweenFramesMapTile((newX * 2) - dirX, (newY * 2) - dirY, TILE_PLAYER);
        gameState.objectGrid[newX][newY].object = TILE_PLAYER;
        SET_STATE(gameState.objectGrid[newX][newY], STATE_STATIONARY);
        
        gameState.playerX = newX;
        gameState.playerY = newY;
        SET_SCANNED(gameState.objectGrid[newX][newY]);
    } else {
        setBothMapTile(newX * 2, newY * 2, TILE_EMPTY);
        gameState.objectGrid[newX][newY].object = TILE_EMPTY;
        SET_STATE(gameState.objectGrid[newX][newY], STATE_STATIONARY);
    }
    gameState.moveCount++;
    if (gameState.physicsWaitingForPlayer) {
        gameState.physicsWaitingForPlayer = FALSE;
        incrementPhysicsPosition();
    }
}

void drawPauseMenu() {
    VDP_clearPlane(BG_B, FALSE);
    VDP_setTextPlane(BG_B);

    VDP_drawText("PAUSED", 15, 10);
    VDP_drawText(gameState.pauseMenuSelection == 0 ? "> Resume" : "  Resume", 14, 12);
    VDP_drawText(gameState.pauseMenuSelection == 1 ? "> Settings" : "  Settings", 14, 13);
    VDP_drawText(gameState.pauseMenuSelection == 2 ? "> Restart" : "  Restart", 14, 14);
    VDP_drawText(gameState.pauseMenuSelection == 3 ? "> Quit" : "  Quit", 14, 15);
}
void unpause() {
    gameState.gameState = GAME_STATE_PLAYING;
    VDP_clearPlane(BG_B, TRUE);
    VDP_setTextPlane(BG_A);
    PAL_setPalette(PAL0, basePalette, DMA);
}
void drawSettingsMenu() {
    char buffer[32];
    VDP_clearPlane(BG_B, FALSE);
    VDP_setTextPlane(BG_B);
    
    VDP_drawText("SETTINGS", 14, 10);
    
    sprintf(buffer, "%c Music: %d", gameState.settingsMenuSelection == 0 ? '>' : ' ', gameState.musicVolume);
    VDP_drawText(buffer, 12, 12);
    
    sprintf(buffer, "%c SFX: %d", gameState.settingsMenuSelection == 1 ? '>' : ' ', gameState.sfxVolume);
    VDP_drawText(buffer, 12, 13);
        
    sprintf(buffer, "%c Repeat Delay: %d", gameState.settingsMenuSelection == 2 ? '>' : ' ', gameState.repeatDelay);
    VDP_drawText(buffer, 12, 14);

    sprintf(buffer, "%c Repeat Rate: %d", gameState.settingsMenuSelection == 3 ? '>' : ' ', gameState.repeatRate);
    VDP_drawText(buffer, 12, 15);

    VDP_drawText(gameState.settingsMenuSelection == 4 ? "> Back" : "  Back", 12, 17);
}

void handlePauseMenuSelect() {
    switch (gameState.pauseMenuSelection) {
        case 0:
            unpause();
            break;
        case 1:
            gameState.gameState = GAME_STATE_SETTINGS;
            gameState.settingsMenuSelection = 0;
            drawSettingsMenu();
            break;
        case 2:
            gameRestartMap();
            unpause();
            redrawStage = REDRAW_STAGE_VISUAL;
            break;
        case 3:
            // quit to main menu - not implemented
            break;
    }
}

void handleSettingsInput(u16 joy, u16 prevJoy) {
    u16 pressed = joy & ~prevJoy;
    
    if (pressed & BUTTON_UP) {
        if (gameState.settingsMenuSelection > 0) gameState.settingsMenuSelection--;
        drawSettingsMenu();
    }
    if (pressed & BUTTON_DOWN) {
        if (gameState.settingsMenuSelection < 4) gameState.settingsMenuSelection++;
        drawSettingsMenu();
    }
    if (pressed & BUTTON_LEFT) {
        if (gameState.settingsMenuSelection == 0 && gameState.musicVolume > 0) {
            gameState.musicVolume--;
            drawSettingsMenu();
        }
        if (gameState.settingsMenuSelection == 1 && gameState.sfxVolume > 0) {
            gameState.sfxVolume--;
            drawSettingsMenu();
        }
        if (gameState.settingsMenuSelection == 2 && gameState.repeatDelay > 0) {
            gameState.repeatDelay = (gameState.repeatDelay > 0) ? gameState.repeatDelay - 1 : 0;
            drawSettingsMenu();
        }
        if (gameState.settingsMenuSelection == 3 && gameState.repeatRate > 0) {
            gameState.repeatRate = (gameState.repeatRate > 0) ? gameState.repeatRate - 1 : 0;
            drawSettingsMenu();
        }

    }
    if (pressed & BUTTON_RIGHT) {
        if (gameState.settingsMenuSelection == 0 && gameState.musicVolume < 10) {
            gameState.musicVolume++;
            drawSettingsMenu();
        }
        if (gameState.settingsMenuSelection == 1 && gameState.sfxVolume < 10) {
            gameState.sfxVolume++;
            drawSettingsMenu();
        }
        if (gameState.settingsMenuSelection == 2 && gameState.repeatDelay < 60) {
            gameState.repeatDelay = (gameState.repeatDelay < 60) ? gameState.repeatDelay + 1 : 60;
            drawSettingsMenu();
        }
        if (gameState.settingsMenuSelection == 3 && gameState.repeatRate < 12) {
            gameState.repeatRate = (gameState.repeatRate < 12) ? gameState.repeatRate + 1 : 12;
            drawSettingsMenu();
        }
    }
    if (pressed & (BUTTON_A | BUTTON_B | BUTTON_START)) {
        if (gameState.settingsMenuSelection == 3 || (pressed & (BUTTON_B | BUTTON_START))) {
            gameState.gameState = GAME_STATE_PAUSED;
            drawPauseMenu();
        }
    }
}



void playerHandleInput()
{
    static u16 prevJoy = 0;
    static u16 repeatTimer = 0;
    static u16 restartTimer = 0;
    
    u16 joy = JOY_readJoypad(JOY_1);
    u16 pressed = joy & ~prevJoy;
    u16 released = prevJoy & ~joy;
    
    if (gameState.gameState == GAME_STATE_PAUSED) {
        if (pressed & BUTTON_START) {
            unpause();
            
        }
        if (pressed & BUTTON_UP) {
            if (gameState.pauseMenuSelection > 0) gameState.pauseMenuSelection--;
            drawPauseMenu();
        }
        if (pressed & BUTTON_DOWN) {
            if (gameState.pauseMenuSelection < 3) gameState.pauseMenuSelection++;
            drawPauseMenu();
        }
        if (pressed & BUTTON_A) {
            handlePauseMenuSelect();
        }
        prevJoy = joy;
        return;
    }
    
    if (gameState.gameState == GAME_STATE_SETTINGS) {
        handleSettingsInput(joy, prevJoy);
        prevJoy = joy;
        return;
    }
    
    prevJoy = joy;
    
    if (released) {
        repeatTimer = 0;
    }
    
    if (joy & BUTTON_C) {
        restartTimer++;
        if (restartTimer >= RESTART_HOLD_TIME) {
            gameRestartMap();
            redrawStage = REDRAW_STAGE_BETWEEN;
            restartTimer = 0;
            return;
        }
    } else {
        restartTimer = 0;
    }
    
    bool shouldProcess = FALSE;
    if (pressed) {
        shouldProcess = TRUE;
        repeatTimer = 0;
    }
    else if (joy & (BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT)) {
        repeatTimer++;
        if (repeatTimer >= gameState.repeatDelay && 
            (repeatTimer - gameState.repeatDelay) % gameState.repeatRate == 0) {
            shouldProcess = TRUE;
        }
    }
    
    if (pressed & BUTTON_START) {
        gameState.gameState = GAME_STATE_PAUSED;
        gameState.pauseMenuSelection = 0;
        PAL_setPalette(PAL0, darkenedPalette, DMA);
        VDP_setTextPalette(PAL1);
        drawPauseMenu();
        return;
    }
    
    if (!shouldProcess) {
        return;
    }
    
    if (pressed & BUTTON_B) {
        gameRewindState();
        redrawStage = REDRAW_STAGE_BETWEEN;
        return;
    }
    
    s8 dx = 0, dy = 0;
    if (joy & BUTTON_UP) dy = -1;
    else if (joy & BUTTON_DOWN) dy = 1;
    else if (joy & BUTTON_LEFT) dx = -1;
    else if (joy & BUTTON_RIGHT) dx = 1;
    
    if (dx != 0 || dy != 0) {
        gameState.playerDirectionX = dx;
        gameState.playerDirectionY = dy;
        gameState.isPushAction = (joy & BUTTON_A) ? TRUE : FALSE;
        gameState.physicsWaitingForPlayer = FALSE;
    }
}
