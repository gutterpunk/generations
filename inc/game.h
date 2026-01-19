#ifndef GAME_H
#define GAME_H

#include <genesis.h>

#define MAX_GRID_SIZE 20
#define MAX_HISTORY 50

#define TILE_EMPTY 0
#define TILE_WALL 1
#define TILE_DIRT 2
#define TILE_BOULDER 3
#define TILE_DIAMOND 4
#define TILE_PLAYER 5

#define STATE_STATIONARY 0
#define STATE_FALLING 1

#define INITIAL_REPEAT_DELAY 20
#define REPEAT_RATE 4 

typedef struct {
    u8 grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
    u8 state[MAX_GRID_SIZE][MAX_GRID_SIZE];
    u8 playerX, playerY;
    s8 playerDirectionX, playerDirectionY;
    u16 moveCount;
    u8 width, height;
    u8 physicsX, physicsY;
    bool physicsWaitingForPlayer;
} GameState;

extern GameState currentState;
extern GameState stateHistory[MAX_HISTORY];
extern u16 historyIndex;
extern bool needsRedraw;

void gameInit();
void gameUpdatePhysics();
void gameSaveState();
void gameRewindState();

#endif
