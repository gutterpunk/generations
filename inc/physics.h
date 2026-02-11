#ifndef PHYSICS_H
#define PHYSICS_H

#include <genesis.h>
#include "game.h"

void handleRollingObjects(ObjectState* tile, u8 x, u8 y, u8 w, u8 h);
void explodeAt(u8 x, u8 y, u8 w, u8 h);

#endif
