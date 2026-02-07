#ifndef MAP_H
#define MAP_H

#include <genesis.h>
#include "game.h"

void mapInit(u8 width, u8 height);
void mapDrawPlayfield(bool inBetweenFrames);
void setVisualMapTile(u8 screenX, u8 screenY, u8 tileType);
void setBetweenFramesMapTile(u8 x, u8 y, u8 tileIndex);
void setBothMapTile(u8 x, u8 y, u8 tileIndex);
#endif
