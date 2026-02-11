/**
 * Creatures Module
 * Handles enemy and creature behaviors
 **/
#include <genesis.h>
#include "game.h"
#include "map.h"
#include "creatures.h"
#include "tiles.h"
#include "physics.h"
#include "player.h"

extern u8 redrawStage;

void handleFirefly(ObjectState* tile, u8 x, u8 y, u8 w, u8 h)
{
    u8 dir = GET_DIR(*tile);
    s8 dx[] = {-1, 0, 1, 0};
    s8 dy[] = {0, -1, 0, 1};
    
    for (u8 checkDir = 0; checkDir < 4; checkDir++) {
        s8 checkX = x + dx[checkDir];
        s8 checkY = y + dy[checkDir];
        if (checkX >= 0 && checkX < w && checkY >= 0 && checkY < h) {
            ObjectState* adjacent = &gameState.objectGrid[(u8)checkX][(u8)checkY];
            if (adjacent->object == TILE_PLAYER || adjacent->object == TILE_BUTTERFLY) {
                explodeAt(x, y, w, h);
                redrawStage = REDRAW_STAGE_BETWEEN;
                return;
            }
        }
    }
    
    u8 leftDir = (dir + 3) % 4;
    s8 signedX = x + dx[leftDir];
    s8 signedY = y + dy[leftDir];
    
    if (signedX >= 0 && signedX < w && signedY >= 0 && signedY < h) {
        u8 leftX = (u8)signedX;
        u8 leftY = (u8)signedY;
        ObjectState* leftTile = &gameState.objectGrid[leftX][leftY];
        if (leftTile->object == TILE_EMPTY) {
            u8 tileType = tile->object;
            setBothMapTile(x * 2, y * 2, TILE_EMPTY);
            tile->object = TILE_EMPTY;
            SET_STATE(*tile, STATE_STATIONARY);
            
            setVisualMapTile(leftX * 2, leftY * 2, tileType);
            setBetweenFramesMapTile((leftX * 2) - dx[leftDir], (leftY * 2) - dy[leftDir], tileType);
            leftTile->object = tileType;
            SET_DIR(*leftTile, leftDir);
            SET_SCANNED(*leftTile);
            redrawStage = REDRAW_STAGE_BETWEEN;
            return;
        }
    }
    
    signedX = x + dx[dir];
    signedY = y + dy[dir];
    
    if (signedX >= 0 && signedX < w && signedY >= 0 && signedY < h) {
        u8 fwdX = (u8)signedX;
        u8 fwdY = (u8)signedY;
        ObjectState* fwdTile = &gameState.objectGrid[fwdX][fwdY];
        if (fwdTile->object == TILE_EMPTY) {
            u8 tileType = tile->object;
            setBothMapTile(x * 2, y * 2, TILE_EMPTY);
            tile->object = TILE_EMPTY;
            SET_STATE(*tile, STATE_STATIONARY);
            
            setVisualMapTile(fwdX * 2, fwdY * 2, tileType);
            setBetweenFramesMapTile((fwdX * 2) - dx[dir], (fwdY * 2) - dy[dir], tileType);
            fwdTile->object = tileType;
            SET_DIR(*fwdTile, dir);
            SET_SCANNED(*fwdTile);
            redrawStage = REDRAW_STAGE_BETWEEN;
            return;
        }
    }
    
    u8 rightDir = (dir + 1) % 4;
    SET_DIR(*tile, rightDir);
}

void updatePlayerPhysics()
{
    u8 newX = gameState.playerX + gameState.playerDirectionX;
    u8 newY = gameState.playerY + gameState.playerDirectionY;
    u8 maxX = gameState.gridWidth;
    u8 maxY = gameState.gridHeight;
    
    if (newX < maxX && newY < maxY) {
        ObjectState* targetTile = &gameState.objectGrid[newX][newY];
        bool isPush = gameState.isPushAction;
        
        if (targetTile->object == TILE_EMPTY || targetTile->object == TILE_DIRT) {
            gameSaveState();
            
            playerMoveOrAction(isPush, newX, newY, gameState.playerDirectionX, gameState.playerDirectionY);
            
            redrawStage = REDRAW_STAGE_BETWEEN;
        }
        else if (targetTile->object == TILE_BOULDER && gameState.playerDirectionY == 0) {
            u8 pushX = newX + gameState.playerDirectionX;
            
            if (pushX < maxX) {
                ObjectState* pushTile = &gameState.objectGrid[pushX][newY];
                
                if (pushTile->object == TILE_EMPTY) {
                    gameSaveState();
                    
                    setVisualMapTile(pushX * 2, newY * 2, TILE_BOULDER);
                    setBetweenFramesMapTile((pushX * 2) - gameState.playerDirectionX, newY * 2, TILE_BOULDER);
                    pushTile->object = TILE_BOULDER;
                    SET_STATE(*pushTile, STATE_STATIONARY);
                    SET_SCANNED(*pushTile);
                    
                    playerMoveOrAction(isPush, newX, newY, gameState.playerDirectionX, gameState.playerDirectionY);
                                                    
                    redrawStage = REDRAW_STAGE_BETWEEN;
                }
            }
        }
        else if (targetTile->object == TILE_DIAMOND) {
            gameSaveState();
            
            setVisualMapTile(newX * 2, newY * 2, TILE_EMPTY);
            setBetweenFramesMapTile(newX * 2, newY * 2, TILE_EMPTY);
            playerMoveOrAction(isPush, newX, newY, gameState.playerDirectionX, gameState.playerDirectionY);
            
            redrawStage = REDRAW_STAGE_BETWEEN; 
        }
    }
}
