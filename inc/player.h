#ifndef PLAYER_H
#define PLAYER_H

#include <genesis.h>
#include "game.h"

void playerHandleInput();
void updatePlayerPhysics();
void playerMoveOrAction(bool isPush, u8 newX, u8 newY, s8 dirX, s8 dirY);
void drawPauseMenu();
void drawSettingsMenu();
void handlePauseMenuSelect();
void handleSettingsInput(u16 joy, u16 prevJoy);

#endif
