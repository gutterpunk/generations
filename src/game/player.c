/**
 * Player Module
 * Handles player input and movement
 **/
#include <genesis.h>
#include "player.h"
#include "game.h"

void playerHandleInput()
{
    static u16 prevJoy = 0;
    static u16 repeatTimer = 0;
    static u16 restartTimer = 0;
    
    u16 joy = JOY_readJoypad(JOY_1);
    u16 pressed = joy & ~prevJoy;
    u16 released = prevJoy & ~joy;
    prevJoy = joy;
    
    if (released) {
        repeatTimer = 0;
    }
    
    if (joy & BUTTON_C) {
        restartTimer++;
        if (restartTimer >= RESTART_HOLD_TIME) {
            gameRestartMap();
            needsRedraw = TRUE;
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
        if (repeatTimer >= INITIAL_REPEAT_DELAY && 
            (repeatTimer - INITIAL_REPEAT_DELAY) % REPEAT_RATE == 0) {
            shouldProcess = TRUE;
        }
    }
    
    if (!shouldProcess) {
        return;
    }
    
    if (pressed & BUTTON_B) {
        gameRewindState();
        needsRedraw = TRUE;
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
