#ifndef GAME_H
#define GAME_H

#include <genesis.h>

#define MAX_GRID_SIZE 20
#define MAX_HISTORY 50

#define TILE_EMPTY 2
#define TILE_WALL 1
#define TILE_DIRT 4
#define TILE_BOULDER 3
#define TILE_DIAMOND 32
#define TILE_PLAYER 16

#define STATE_STATIONARY 0
#define STATE_FALLING 1

#define INITIAL_REPEAT_DELAY 8
#define REPEAT_RATE 2
#define RESTART_HOLD_TIME 60

typedef struct {
    u8 original[MAX_GRID_SIZE][MAX_GRID_SIZE];
    u8 grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
    u8 state[MAX_GRID_SIZE][MAX_GRID_SIZE];
    u8 playerX, playerY;
    s8 playerDirectionX, playerDirectionY;
    u16 moveCount;
    u8 width, height;
    u8 physicsX, physicsY;
    bool physicsWaitingForPlayer;
    bool isPushAction;

    //interim frame, object to draw is placed here before rendering from the engine loop, at half movement
    u8 interim[MAX_GRID_SIZE * 2][MAX_GRID_SIZE * 2]; 

    u8 history[MAX_HISTORY][MAX_GRID_SIZE][MAX_GRID_SIZE];
    u16 historyIndex;
} GameState;

extern GameState gameState;
extern bool needsRedraw;
extern u32 frameCounter;

void gameInit();
void gameUpdatePhysics();
void gameSaveState();
void gameRewindState();
void gameRestartMap();

#endif
