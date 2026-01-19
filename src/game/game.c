/**
 * Game Logic Module
 * Handles game state and physics
 **/
#include <genesis.h>
#include "game.h"
#include "map.h"
#include "player.h"

// Global game state
GameState gameState;
GameState stateHistory[MAX_HISTORY];
u16 historyIndex = 0;
bool needsRedraw = TRUE;

u8 physicsProcessed[MAX_GRID_SIZE][MAX_GRID_SIZE];

void gameInit()
{
    mapInit(10, 10);
    gameState.physicsX = 0;
    gameState.physicsY = 0;
    gameState.physicsWaitingForPlayer = FALSE;
    gameState.isPushAction = FALSE;
    needsRedraw = TRUE;
}

static void incrementPhysicsPosition()
{
    u8 w = gameState.width;
    u8 h = gameState.height;
    
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

static bool isRounded(u8 tile, u8 state)
{
    if (tile == TILE_WALL) return TRUE;
    if (tile == TILE_BOULDER && state == STATE_STATIONARY) return TRUE;
    if (tile == TILE_DIAMOND && state == STATE_STATIONARY) return TRUE;
    return FALSE;
}

static bool tryRoll(u8 x, u8 y, u8 tile)
{
    u8 w = gameState.width;
    u8 h = gameState.height;
    
    if (x > 0 && y < h - 1) {
        if (gameState.grid[x - 1][y] == TILE_EMPTY &&
            gameState.grid[x - 1][y + 1] == TILE_EMPTY) {
            gameState.grid[x][y] = TILE_EMPTY;
            gameState.state[x][y] = STATE_STATIONARY;
            gameState.grid[x - 1][y] = tile;
            gameState.state[x - 1][y] = STATE_FALLING;
            physicsProcessed[x - 1][y] = 1;
            needsRedraw = TRUE;
            return TRUE;
        }
    }
    
    if (x < w - 1 && y < h - 1) {
        if (gameState.grid[x + 1][y] == TILE_EMPTY &&
            gameState.grid[x + 1][y + 1] == TILE_EMPTY) {
            gameState.grid[x][y] = TILE_EMPTY;
            gameState.state[x][y] = STATE_STATIONARY;
            gameState.grid[x + 1][y] = tile;
            gameState.state[x + 1][y] = STATE_FALLING;
            physicsProcessed[x + 1][y] = 1;
            needsRedraw = TRUE;
            return TRUE;
        }
    }
    
    return FALSE;
}
static void playerMoveOrAction(bool isPush, u8 newX, u8 newY) 
{
    if (!isPush) {
        gameState.grid[gameState.playerX][gameState.playerY] = TILE_EMPTY;
        gameState.grid[newX][newY] = TILE_PLAYER;
        physicsProcessed[newX][newY] = 1;
    } else {
        gameState.grid[newX][newY] = TILE_EMPTY;
    }
    gameState.moveCount++;
    if (gameState.physicsWaitingForPlayer) {
        gameState.physicsWaitingForPlayer = FALSE;
        incrementPhysicsPosition();
    }

}
static void updatePlayerPhysics()
{
    u8 newX = gameState.playerX + gameState.playerDirectionX;
    u8 newY = gameState.playerY + gameState.playerDirectionY;
    
    if (newX < gameState.width && newY < gameState.height) {
        u8 targetTile = gameState.grid[newX][newY];
        bool isPush = gameState.isPushAction;
        
        if (targetTile == TILE_EMPTY || targetTile == TILE_DIRT) {
            gameSaveState();
            
            playerMoveOrAction(isPush, newX, newY);
            
            needsRedraw = TRUE;
        }
        else if (targetTile == TILE_BOULDER && gameState.playerDirectionY == 0) {
            u8 pushX = newX + gameState.playerDirectionX;
            
            if (pushX < gameState.width &&
                gameState.grid[pushX][newY] == TILE_EMPTY) {
                gameSaveState();
                
                gameState.grid[pushX][newY] = TILE_BOULDER;
                gameState.state[pushX][newY] = STATE_STATIONARY;
                physicsProcessed[pushX][newY] = 1;
                
                playerMoveOrAction(isPush, newX, newY);
                                                
                needsRedraw = TRUE;
            }
        }
    }
}

void gameUpdatePhysics()
{
    u8 w = gameState.width;
    u8 h = gameState.height;
    
    if (gameState.physicsWaitingForPlayer) {
        return;
    }
    
    if (gameState.physicsX == 0 && gameState.physicsY == 0) {
        memset(physicsProcessed, 0, MAX_GRID_SIZE * MAX_GRID_SIZE);
    }
    
    u8 startX = gameState.physicsX;
    for (u8 y = gameState.physicsY; y < h; y++) {
        for (u8 x = startX; x < w; x++) {
            if (physicsProcessed[x][y]) {
                continue;
            }
            
            u8 tile = gameState.grid[x][y];

            if (tile == TILE_PLAYER) {
                gameState.playerX = x;
                gameState.playerY = y;
                if (gameState.playerDirectionX != 0 || gameState.playerDirectionY != 0) {
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
            
            u8 state = gameState.state[x][y];
            
            if (tile == TILE_BOULDER || tile == TILE_DIAMOND) {
                if (y >= h - 1) continue;
                
                u8 below = gameState.grid[x][y + 1];
                u8 belowState = gameState.state[x][y + 1];
                
                if (state == STATE_STATIONARY) {
                    if (below == TILE_EMPTY) {
                        gameState.grid[x][y] = TILE_EMPTY;
                        gameState.state[x][y] = STATE_STATIONARY;
                        gameState.grid[x][y + 1] = tile;
                        gameState.state[x][y + 1] = STATE_FALLING;
                        physicsProcessed[x][y + 1] = 1;
                        needsRedraw = TRUE;
                    }
                    else if (isRounded(below, belowState)) {
                        tryRoll(x, y, tile);
                    }
                }
                else {
                    if (below == TILE_EMPTY) {
                        gameState.grid[x][y] = TILE_EMPTY;
                        gameState.state[x][y] = STATE_STATIONARY;
                        gameState.grid[x][y + 1] = tile;
                        gameState.state[x][y + 1] = STATE_FALLING;
                        physicsProcessed[x][y + 1] = 1;
                        needsRedraw = TRUE;
                    }
                    else {
                        if (isRounded(below, belowState)) {
                            if (!tryRoll(x, y, tile)) {
                                gameState.state[x][y] = STATE_STATIONARY;
                            }
                        }
                        else if (below == TILE_PLAYER) {
                            gameState.state[x][y] = STATE_STATIONARY;
                        }
                        else {
                            gameState.state[x][y] = STATE_STATIONARY;
                        }
                    }
                }
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
        memcpy(&gameState.history[gameState.historyIndex], &gameState.grid, sizeof(gameState.grid));
        gameState.historyIndex++;
    } else {
        for (u16 i = 1; i < MAX_HISTORY; i++) {
            memcpy(&gameState.history[i - 1], &gameState.history[i], sizeof(gameState.grid));
        }
        memcpy(&gameState.history[MAX_HISTORY - 1], &gameState.grid, sizeof(gameState.grid));
    }
}

void gameRewindState()
{
    if (gameState.historyIndex > 1) {
        memcpy(&gameState.grid, &gameState.history[gameState.historyIndex - 1], sizeof(gameState.grid));
        
        gameState.historyIndex--;
    }
}

void gameRestartMap()
{
    gameState.historyIndex = 1;
    memcpy(&gameState.grid, &gameState.original, sizeof(gameState.grid));
}
