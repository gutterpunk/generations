/**
 * Physics Module
 * Handles falling and rolling object mechanics
 **/
#include <genesis.h>
#include "game.h"
#include "map.h"
#include "physics.h"
#include "tiles.h"

extern u8 redrawStage;

static bool isRounded(u8 tile, u8 state)
{
    if (tile == TILE_WALL) return TRUE;
    if (tile == TILE_BOULDER && state == STATE_STATIONARY) return TRUE;
    if (tile == TILE_DIAMOND && state == STATE_STATIONARY) return TRUE;
    return FALSE;
}

static bool tryRollLeft(u8 x, u8 y, u8 tile)
{
    u8 h = gameState.gridHeight;
    if (x > 0 && y < h - 1) {
        u8 leftX = x - 1;
        u8 belowY = y + 1;
        ObjectState* leftTileType = &gameState.objectGrid[leftX][y];
        ObjectState* belowLeftTileType = &gameState.objectGrid[leftX][belowY];
        if (leftTileType->object == TILE_EMPTY && belowLeftTileType->object == TILE_EMPTY) {
            setVisualMapTile(x * 2, y * 2, TILE_EMPTY);
            setBetweenFramesMapTile(x * 2, y * 2, TILE_EMPTY);
            gameState.objectGrid[x][y].object = TILE_EMPTY;
            SET_STATE(gameState.objectGrid[x][y], STATE_STATIONARY);
            
            setVisualMapTile(leftX * 2, y * 2, tile);
            setBetweenFramesMapTile((leftX * 2) + 1, y * 2, tile);
            leftTileType->object = tile;
            SET_STATE(*leftTileType, STATE_FALLING);
            SET_SCANNED(*leftTileType);
            redrawStage = REDRAW_STAGE_BETWEEN;
            return TRUE;
        }
    }
    return FALSE;
}

static bool tryRollRight(u8 x, u8 y, u8 tile)
{
    u8 w = gameState.gridWidth;
    u8 h = gameState.gridHeight;
    
    if (x < w - 1 && y < h - 1) {
        u8 rightX = x + 1;
        u8 belowY = y + 1;
        ObjectState* rightTileType = &gameState.objectGrid[rightX][y];
        ObjectState* belowRightTileType = &gameState.objectGrid[rightX][belowY];
        if (rightTileType->object == TILE_EMPTY && belowRightTileType->object == TILE_EMPTY) {
            setBothMapTile(x * 2, y * 2, TILE_EMPTY);
            gameState.objectGrid[x][y].object = TILE_EMPTY;
            SET_STATE(gameState.objectGrid[x][y], STATE_STATIONARY);
            
            setVisualMapTile(rightX * 2, y * 2, tile);
            setBetweenFramesMapTile((rightX * 2) - 1, y * 2, tile);
            rightTileType->object = tile;
            SET_STATE(*rightTileType, STATE_FALLING);
            SET_SCANNED(*rightTileType);
            redrawStage = REDRAW_STAGE_BETWEEN;
            return TRUE;
        }
    }
    return FALSE;
}

static bool tryRoll(u8 x, u8 y, u8 tile, bool leftFirst)
{
    if (leftFirst) {
        if (tryRollLeft(x, y, tile)) return TRUE;
        if (tryRollRight(x, y, tile)) return TRUE;
    } else {
        if (tryRollRight(x, y, tile)) return TRUE;
        if (tryRollLeft(x, y, tile)) return TRUE;
    }    
    return FALSE;
}

void handleRollingObjects(ObjectState* tile, u8 x, u8 y, u8 w, u8 h)
{
    ObjectState* below = NULL;
    u8 belowObject = TILE_WALL;
    u8 belowState = STATE_STATIONARY;
    
    if (y < h - 1) {
        below = &gameState.objectGrid[x][y + 1];
        belowObject = below->object;
        belowState = GET_STATE(*below);
    }

    if (GET_STATE(*tile) == STATE_STATIONARY) {
        if (belowObject == TILE_EMPTY) {
            u8 tileType = tile->object;
            
            setBothMapTile(x * 2, y * 2, TILE_EMPTY);
            tile->object = TILE_EMPTY;
            SET_STATE(*tile, STATE_STATIONARY);
            
            u8 newY = y + 1;
            setVisualMapTile(x * 2, newY * 2, tileType);
            setBetweenFramesMapTile(x * 2, (newY * 2) - 1, tileType);
            below->object = tileType;
            SET_STATE(*below, STATE_FALLING);
            SET_SCANNED(*below);
            redrawStage = REDRAW_STAGE_BETWEEN;
        }
        else if (isRounded(belowObject, belowState)) {
            tryRoll(x, y, tile->object, tile->object == TILE_BOULDER);
        }
    }
    else {
        if (belowObject == TILE_EMPTY) {
            u8 tileType = tile->object;
            setBothMapTile(x * 2, y * 2, TILE_EMPTY);
            tile->object = TILE_EMPTY;
            SET_STATE(*tile, STATE_STATIONARY);
            
            u8 newY = y + 1;
            setVisualMapTile(x * 2, newY * 2, tileType);
            setBetweenFramesMapTile(x * 2, (newY * 2) - 1, tileType);
            below->object = tileType;
            SET_STATE(*below, STATE_FALLING);
            SET_SCANNED(*below);
            redrawStage = REDRAW_STAGE_BETWEEN;
        }
        else {
            if (isRounded(belowObject, belowState)) {
                if (!tryRoll(x, y, tile->object, tile->object == TILE_BOULDER)) {
                    SET_STATE(*tile, STATE_STATIONARY);
                }
            }
            else {
                SET_STATE(*tile, STATE_STATIONARY);
            }
        }
    }
}

void explodeAt(u8 x, u8 y, u8 w, u8 h)
{
    for (s8 ex = -1; ex <= 1; ex++) {
        for (s8 ey = -1; ey <= 1; ey++) {
            s8 explodeX = x + ex;
            s8 explodeY = y + ey;
            if (explodeX >= 0 && explodeX < w && explodeY >= 0 && explodeY < h) {
                setBothMapTile(explodeX * 2, explodeY * 2, TILE_EMPTY);
                gameState.objectGrid[(u8)explodeX][(u8)explodeY].object = TILE_EMPTY;
                SET_STATE(gameState.objectGrid[(u8)explodeX][(u8)explodeY], STATE_STATIONARY);
                SET_SCANNED(gameState.objectGrid[(u8)explodeX][(u8)explodeY]);
            }
        }
    }
}
