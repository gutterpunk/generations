#ifndef GAME_H
#define GAME_H

#include <genesis.h>

#define MAX_GRID_SIZE 14
#define MAX_HISTORY 50

#define STATE_STATIONARY 0
#define STATE_FALLING 1
#define STATE_SCANNED 2

#define DIR_LEFT 0
#define DIR_UP 1
#define DIR_RIGHT 2
#define DIR_DOWN 3

#define STATE_MASK 0x03
#define DIR_SHIFT 2
#define DIR_MASK 0x03

#define GET_STATE(objState) ((objState).state & 0x01)
#define GET_DIR(objState) (((objState).state >> DIR_SHIFT) & DIR_MASK)
#define SET_STATE(objState, s) ((objState).state = ((objState).state & ~0x01) | (s))
#define SET_DIR(objState, d) ((objState).state = ((objState).state & STATE_MASK) | ((d) << DIR_SHIFT))
#define SET_SCANNED(objState) ((objState).state |= STATE_SCANNED)
#define CLEAR_SCANNED(objState) ((objState).state &= ~STATE_SCANNED)
#define IS_SCANNED(objState) ((objState).state & STATE_SCANNED)

#define INITIAL_REPEAT_DELAY 30
#define REPEAT_RATE 10
#define RESTART_HOLD_TIME 60

#define REDRAW_STAGE_NONE 0
#define REDRAW_STAGE_BETWEEN 1
#define REDRAW_STAGE_VISUAL 2

#define GAME_STATE_PLAYING 0
#define GAME_STATE_PAUSED 1
#define GAME_STATE_SETTINGS 2

typedef struct  {
    u8 object;
    u8 state;
} ObjectState;

typedef struct {
    u8 gridWidth, gridHeight;
    u8 gameState;
    u8 pauseMenuSelection;
    u8 settingsMenuSelection;
    u8 musicVolume;
    u8 sfxVolume;
    u8 repeatDelay;
    u8 repeatRate;

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
extern u8 redrawStage;
extern u32 frameCounter;

void gameInit();
void gameUpdatePhysics();
void gameSaveState();
void gameRewindState();
void gameRestartMap();

#endif
