#ifndef MAP_H
#define MAP_H

#include <genesis.h>
#include "game.h"

void mapInit(u8 width, u8 height);
void mapDrawPlayfield();
void setVisualMapTile(u8 screenX, u8 screenY, u8 tileType);

#endif
