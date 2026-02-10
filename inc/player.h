#ifndef PLAYER_H
#define PLAYER_H

#include <genesis.h>

void playerHandleInput();
void drawPauseMenu();
void drawSettingsMenu();
void handlePauseMenuSelect();
void handleSettingsInput(u16 joy, u16 prevJoy);

#endif
