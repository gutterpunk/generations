/**
 * Game Logic Module
 * Handles game state and physics orchestration
 **/
#include <genesis.h>
#include "game.h"
#include "map.h"
#include "player.h"
#include "physics.h"
#include "creatures.h"
#include "tiles.h"



GameState gameState;
u8 redrawStage = REDRAW_STAGE_NONE;

void gameInit()
{
    mapInit(4); 
    gameState.physicsX = 0;
    gameState.physicsY = 0;
    gameState.physicsWaitingForPlayer = FALSE;
    gameState.isPushAction = FALSE;    gameState.gameState = GAME_STATE_PLAYING;
    gameState.pauseMenuSelection = 0;
    gameState.settingsMenuSelection = 0;
    gameState.musicVolume = 5;
    gameState.sfxVolume = 5;
    gameState.repeatDelay = INITIAL_REPEAT_DELAY;
    gameState.repeatRate = REPEAT_RATE;
    redrawStage = REDRAW_STAGE_VISUAL;
}

void incrementPhysicsPosition()
{
    u8 w = gameState.gridWidth;
    u8 h = gameState.gridHeight;
    
    u8 x = gameState.physicsX;
    u8 y = gameState.physicsY;
    
    x++;
    if (x >= w) {
        x = 0;
        y++;
        if (y >= h) {
            x = 0;
            y = 0;
        }
    }
    
    gameState.physicsX = x;
    gameState.physicsY = y;
}

void gameUpdatePhysics()
{
    u8 w = gameState.gridWidth;   
    u8 h = gameState.gridHeight;   
    
    if (gameState.physicsWaitingForPlayer) {
        return;
    }
    
    if (gameState.physicsX == 0 && gameState.physicsY == 0) {
        for (u8 cy = 0; cy < h; cy++) {
            for (u8 cx = 0; cx < w; cx++) {
                CLEAR_SCANNED(gameState.objectGrid[cx][cy]);
            }
        }
        memcpy(&gameState.betweenFramesGrid, &gameState.visualGrid, MAX_GRID_SIZE * 2 * MAX_GRID_SIZE * 2);
    }
    
    u8 startX = gameState.physicsX;
    for (u8 y = gameState.physicsY; y < h; y++) {
        for (u8 x = startX; x < w; x++) {
            if (IS_SCANNED(gameState.objectGrid[x][y])) {
                continue;
            }            
            ObjectState* tile = &gameState.objectGrid[x][y];

            if (tile->object == TILE_PLAYER) {
                gameState.playerX = x;
                gameState.playerY = y;
                if (gameState.playerDirectionX != 0 || gameState.playerDirectionY != 0) {
                    memcpy(&gameState.betweenFramesGrid, &gameState.visualGrid, MAX_GRID_SIZE * 2 * MAX_GRID_SIZE * 2);
                    updatePlayerPhysics();
                    gameState.playerDirectionX = 0;
                    gameState.playerDirectionY = 0;
                } else {
                    gameState.physicsX = x;
                    gameState.physicsY = y;
                    gameState.playerDirectionX = 0;
                    gameState.playerDirectionY = 0;
                    gameState.physicsWaitingForPlayer = TRUE;
                    return;
                }
            }
            switch (tile->object) {
                case TILE_BOULDER:
                case TILE_DIAMOND:
                    handleRollingObjects(tile, x, y, w, h);
                    break;
                
                case TILE_FIREFLY:
                    handleFirefly(tile, x, y, w, h);
                    break;
                
                default:
                    continue;
            }
        }
        startX = 0;
    }
    
    gameState.physicsX = 0;
    gameState.physicsY = 0;
}

void gameSaveState()
{
    if (gameState.historyIndex < MAX_HISTORY) {
        memcpy(&gameState.history[gameState.historyIndex], &gameState.objectGrid, sizeof(gameState.objectGrid));
        gameState.historyIndex++;
    } else {
        for (u16 i = 1; i < MAX_HISTORY; i++) {
            memcpy(&gameState.history[i - 1], &gameState.history[i], sizeof(gameState.objectGrid));
        }
        memcpy(&gameState.history[MAX_HISTORY - 1], &gameState.objectGrid, sizeof(gameState.objectGrid));
    }
}

void gameRewindState()
{
    if (gameState.historyIndex > 1) {
        gameState.historyIndex--;
        memcpy(&gameState.objectGrid, &gameState.history[gameState.historyIndex - 1], sizeof(gameState.objectGrid));
        
        // Rebuild visualGrid from objectGrid
        for (u8 x = 0; x < gameState.gridWidth; x++) {
            for (u8 y = 0; y < gameState.gridHeight; y++) {
                setVisualMapTile(x * 2, y * 2, gameState.objectGrid[x][y].object);
            }
        }
        redrawStage = REDRAW_STAGE_BETWEEN;
    }
}

void gameRestartMap()
{
    gameState.historyIndex = 1;
    memcpy(&gameState.objectGrid, &gameState.original, sizeof(gameState.objectGrid));
    
    // Rebuild visualGrid from objectGrid
    for (u8 x = 0; x < gameState.gridWidth; x++) {
        for (u8 y = 0; y < gameState.gridHeight; y++) {
            setVisualMapTile(x * 2, y * 2, gameState.objectGrid[x][y].object);
        }
    }
    redrawStage = REDRAW_STAGE_BETWEEN;
}

