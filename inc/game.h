#ifndef GAME_H
#define GAME_H

#include <genesis.h>

#define MAX_GRID_SIZE 14
#define MAX_HISTORY 50

#define STATE_STATIONARY 0
#define STATE_SCANNED 1
#define STATE_FALLING 2

#define INITIAL_REPEAT_DELAY 8
#define REPEAT_RATE 2
#define RESTART_HOLD_TIME 60

typedef struct  {
    u8 object;
    u8 state;
} ObjectState;

typedef struct {
    u8 gridWidth, gridHeight;

    ObjectState original[MAX_GRID_SIZE][MAX_GRID_SIZE];
    u8 visualGrid[MAX_GRID_SIZE * 2][MAX_GRID_SIZE * 2];
    u8 betweenFramesGrid[MAX_GRID_SIZE * 2][MAX_GRID_SIZE * 2];
    ObjectState objectGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
    u8 playerX, playerY;
    s8 playerDirectionX, playerDirectionY;
    bool isPushAction;
    u16 moveCount;

    u8 physicsX, physicsY;
    bool physicsWaitingForPlayer;

    ObjectState history[MAX_HISTORY][MAX_GRID_SIZE][MAX_GRID_SIZE];
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
