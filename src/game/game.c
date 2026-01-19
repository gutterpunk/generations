/**
 * Game Logic Module
 * Handles game state and physics
 **/
#include <genesis.h>
#include "game.h"
#include "map.h"
#include "player.h"

// Global game state
GameState currentState;
GameState stateHistory[MAX_HISTORY];
u16 historyIndex = 0;
bool needsRedraw = TRUE;

u8 physicsProcessed[MAX_GRID_SIZE][MAX_GRID_SIZE];

void gameInit()
{
    mapInit(10, 10);
    currentState.physicsX = 0;
    currentState.physicsY = 0;
    currentState.physicsWaitingForPlayer = FALSE;
    needsRedraw = TRUE;
}

static void incrementPhysicsPosition()
{
    u8 w = currentState.width;
    u8 h = currentState.height;
    
    u8 x = currentState.physicsX;
    u8 y = currentState.physicsY;
    
    x++;
    if (x >= w) {
        x = 0;
        y++;
        if (y >= h) {
            x = 0;
            y = 0;
        }
    }
    
    currentState.physicsX = x;
    currentState.physicsY = y;
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
    u8 w = currentState.width;
    u8 h = currentState.height;
    
    if (x > 0 && y < h - 1) {
        if (currentState.grid[x - 1][y] == TILE_EMPTY &&
            currentState.grid[x - 1][y + 1] == TILE_EMPTY) {
            currentState.grid[x][y] = TILE_EMPTY;
            currentState.state[x][y] = STATE_STATIONARY;
            currentState.grid[x - 1][y] = tile;
            currentState.state[x - 1][y] = STATE_FALLING;
            physicsProcessed[x - 1][y] = 1;
            needsRedraw = TRUE;
            return TRUE;
        }
    }
    
    if (x < w - 1 && y < h - 1) {
        if (currentState.grid[x + 1][y] == TILE_EMPTY &&
            currentState.grid[x + 1][y + 1] == TILE_EMPTY) {
            currentState.grid[x][y] = TILE_EMPTY;
            currentState.state[x][y] = STATE_STATIONARY;
            currentState.grid[x + 1][y] = tile;
            currentState.state[x + 1][y] = STATE_FALLING;
            physicsProcessed[x + 1][y] = 1;
            needsRedraw = TRUE;
            return TRUE;
        }
    }
    
    return FALSE;
}

static void updatePlayerPhysics()
{
    u8 newX = currentState.playerX + currentState.playerDirectionX;
    u8 newY = currentState.playerY + currentState.playerDirectionY;
    
    if (newX < currentState.width && newY < currentState.height) {
        u8 targetTile = currentState.grid[newX][newY];
        
        if (targetTile == TILE_EMPTY || targetTile == TILE_DIRT) {
            gameSaveState();
            
            currentState.grid[currentState.playerX][currentState.playerY] = TILE_EMPTY;
            currentState.playerX = newX;
            currentState.playerY = newY;
            currentState.grid[newX][newY] = TILE_PLAYER;
            physicsProcessed[newX][newY] = 1;
            currentState.moveCount++;
            
            if (currentState.physicsWaitingForPlayer) {
                currentState.physicsWaitingForPlayer = FALSE;
                incrementPhysicsPosition();
            }
            needsRedraw = TRUE;
        }
        else if (targetTile == TILE_BOULDER && currentState.playerDirectionY == 0) {
            u8 pushX = newX + currentState.playerDirectionX;
            
            if (pushX < currentState.width &&
                currentState.grid[pushX][newY] == TILE_EMPTY) {
                gameSaveState();
                
                currentState.grid[pushX][newY] = TILE_BOULDER;
                currentState.state[pushX][newY] = STATE_STATIONARY;
                
                currentState.grid[currentState.playerX][currentState.playerY] = TILE_EMPTY;
                currentState.playerX = newX;
                currentState.playerY = newY;
                currentState.grid[newX][newY] = TILE_PLAYER;
                physicsProcessed[newX][newY] = 1;
                physicsProcessed[pushX][newY] = 1;
                currentState.moveCount++;
                
                if (currentState.physicsWaitingForPlayer) {
                    currentState.physicsWaitingForPlayer = FALSE;
                    incrementPhysicsPosition();
                }
                
                needsRedraw = TRUE;
            }
        }
    }
}

void gameUpdatePhysics()
{
    u8 w = currentState.width;
    u8 h = currentState.height;
    
    if (currentState.physicsWaitingForPlayer) {
        return;
    }
    
    if (currentState.physicsX == 0 && currentState.physicsY == 0) {
        memset(physicsProcessed, 0, MAX_GRID_SIZE * MAX_GRID_SIZE);
    }
    
    u8 startX = currentState.physicsX;
    for (u8 y = currentState.physicsY; y < h; y++) {
        for (u8 x = startX; x < w; x++) {
            if (physicsProcessed[x][y]) {
                continue;
            }
            
            if (x == currentState.playerX && y == currentState.playerY) {
                if (currentState.playerDirectionX != 0 || currentState.playerDirectionY != 0) {
                    updatePlayerPhysics();
                    currentState.playerDirectionX = 0;
                    currentState.playerDirectionY = 0;
                } else {
                    currentState.physicsX = x;
                    currentState.physicsY = y;
                    currentState.playerDirectionX = 0;
                    currentState.playerDirectionY = 0;
                    currentState.physicsWaitingForPlayer = TRUE;
                    return;
                }
            }
            
            u8 tile = currentState.grid[x][y];
            u8 state = currentState.state[x][y];
            
            if (tile == TILE_BOULDER || tile == TILE_DIAMOND) {
                if (y >= h - 1) continue;
                
                u8 below = currentState.grid[x][y + 1];
                u8 belowState = currentState.state[x][y + 1];
                
                if (state == STATE_STATIONARY) {
                    if (below == TILE_EMPTY) {
                        currentState.grid[x][y] = TILE_EMPTY;
                        currentState.state[x][y] = STATE_STATIONARY;
                        currentState.grid[x][y + 1] = tile;
                        currentState.state[x][y + 1] = STATE_FALLING;
                        physicsProcessed[x][y + 1] = 1;
                        needsRedraw = TRUE;
                    }
                    else if (isRounded(below, belowState)) {
                        tryRoll(x, y, tile);
                    }
                }
                else {
                    if (below == TILE_EMPTY) {
                        currentState.grid[x][y] = TILE_EMPTY;
                        currentState.state[x][y] = STATE_STATIONARY;
                        currentState.grid[x][y + 1] = tile;
                        currentState.state[x][y + 1] = STATE_FALLING;
                        physicsProcessed[x][y + 1] = 1;
                        needsRedraw = TRUE;
                    }
                    else {
                        if (isRounded(below, belowState)) {
                            if (!tryRoll(x, y, tile)) {
                                currentState.state[x][y] = STATE_STATIONARY;
                            }
                        }
                        else if (below == TILE_PLAYER) {
                            currentState.state[x][y] = STATE_STATIONARY;
                        }
                        else {
                            currentState.state[x][y] = STATE_STATIONARY;
                        }
                    }
                }
            }
        }
        startX = 0;
    }
    
    currentState.physicsX = 0;
    currentState.physicsY = 0;
}

void gameSaveState()
{
    if (historyIndex < MAX_HISTORY) {
        memcpy(&stateHistory[historyIndex], &currentState, sizeof(GameState));
        historyIndex++;
    }
}

void gameRewindState()
{
    if (historyIndex > 1) {
        historyIndex--;
        memcpy(&currentState, &stateHistory[historyIndex - 1], sizeof(GameState));
    }
}
