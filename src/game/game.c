/**
 * Game Logic Module
 * Handles game state and physics
 **/
#include <genesis.h>
#include "game.h"
#include "map.h"
#include "player.h"
#include "tiles.h"



GameState gameState;
u8 redrawStage = REDRAW_STAGE_NONE;

void gameInit()
{
    mapInit(4); 
    gameState.physicsX = 0;
    gameState.physicsY = 0;
    gameState.physicsWaitingForPlayer = FALSE;
    gameState.isPushAction = FALSE;    gameState.gameState = GAME_STATE_PLAYING;
    gameState.pauseMenuSelection = 0;
    gameState.settingsMenuSelection = 0;
    gameState.musicVolume = 5;
    gameState.sfxVolume = 5;
    gameState.repeatDelay = INITIAL_REPEAT_DELAY;
    gameState.repeatRate = REPEAT_RATE;
    redrawStage = REDRAW_STAGE_VISUAL;
}

static void incrementPhysicsPosition()
{
    u8 w = gameState.gridWidth;
    u8 h = gameState.gridHeight;
    
    u8 x = gameState.physicsX;
    u8 y = gameState.physicsY;
    
    x++;
    if (x >= w) {
        x = 0;
        y++;
        if (y >= h) {
            x = 0;
            y = 0;
        }
    }
    
    gameState.physicsX = x;
    gameState.physicsY = y;
}

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
    // Try roll left
    if (x > 0 && y < h - 1) {
        u8 leftX = x - 1;
        u8 belowY = y + 1;
        ObjectState* leftTileType = &gameState.objectGrid[leftX][y];
        ObjectState* belowLeftTileType = &gameState.objectGrid[leftX][belowY];
        if (leftTileType->object == TILE_EMPTY && belowLeftTileType->object == TILE_EMPTY) {
            // Clear old position
            setVisualMapTile(x * 2, y * 2, TILE_EMPTY);
            setBetweenFramesMapTile(x * 2, y * 2, TILE_EMPTY);
            gameState.objectGrid[x][y].object = TILE_EMPTY;
            SET_STATE(gameState.objectGrid[x][y], STATE_STATIONARY);
            
            // Set new position
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
        
    // Try roll right
    if (x < w - 1 && y < h - 1) {
        u8 rightX = x + 1;
        u8 belowY = y + 1;
        ObjectState* rightTileType = &gameState.objectGrid[rightX][y];
        ObjectState* belowRightTileType = &gameState.objectGrid[rightX][belowY];
        if (rightTileType->object == TILE_EMPTY && belowRightTileType->object == TILE_EMPTY) {
            // Clear old position
            setBothMapTile(x * 2, y * 2, TILE_EMPTY);
            gameState.objectGrid[x][y].object = TILE_EMPTY;
            SET_STATE(gameState.objectGrid[x][y], STATE_STATIONARY);
            
            // Set new position
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
        if (tryRollLeft(x, y, tile)) {
            return TRUE;
        }
        if (tryRollRight(x, y, tile)) {
            return TRUE;
        }
    } else {
        if (tryRollRight(x, y, tile)) {
            return TRUE;
        }
        if (tryRollLeft(x, y, tile)) {
            return TRUE;
        }
    }    
    return FALSE;
}

static void playerMoveOrAction(bool isPush, u8 newX, u8 newY, s8 dirX, s8 dirY) 
{
    if (!isPush) {
        // Clear old player position
        setBothMapTile(gameState.playerX * 2, gameState.playerY * 2, TILE_EMPTY);
        gameState.objectGrid[gameState.playerX][gameState.playerY].object = TILE_EMPTY;
        SET_STATE(gameState.objectGrid[gameState.playerX][gameState.playerY], STATE_STATIONARY);
        
        // Set new player position
        setVisualMapTile(newX * 2, newY * 2, TILE_PLAYER);
        setBetweenFramesMapTile((newX * 2) - dirX, (newY * 2) - dirY, TILE_PLAYER);
        gameState.objectGrid[newX][newY].object = TILE_PLAYER;
        SET_STATE(gameState.objectGrid[newX][newY], STATE_STATIONARY);
        
        gameState.playerX = newX;
        gameState.playerY = newY;
        SET_SCANNED(gameState.objectGrid[newX][newY]);
    } else {
        // Clear area for push action
        setBothMapTile(newX * 2, newY * 2, TILE_EMPTY);
        gameState.objectGrid[newX][newY].object = TILE_EMPTY;
        SET_STATE(gameState.objectGrid[newX][newY], STATE_STATIONARY);
    }
    gameState.moveCount++;
    if (gameState.physicsWaitingForPlayer) {
        gameState.physicsWaitingForPlayer = FALSE;
        incrementPhysicsPosition();
    }

}
static void updatePlayerPhysics()
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
                    
                    // Move boulder
                    setVisualMapTile(pushX * 2, newY * 2, TILE_BOULDER);
                    setBetweenFramesMapTile((pushX * 2) - gameState.playerDirectionX, newY * 2, TILE_BOULDER);
                    pushTile->object = TILE_BOULDER;
                    SET_STATE(*pushTile, STATE_STATIONARY);
                    SET_SCANNED(*pushTile);
                    
                    // Player moves into boulder's old position
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
            // Save tile type before clearing
            u8 tileType = tile->object;
            
            // Clear old position
            setBothMapTile(x * 2, y * 2, TILE_EMPTY);
            tile->object = TILE_EMPTY;
            SET_STATE(*tile, STATE_STATIONARY);
            
            // Set new position
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
            // Clear old position
            u8 tileType = tile->object;
            setBothMapTile(x * 2, y * 2, TILE_EMPTY);
            tile->object = TILE_EMPTY;
            SET_STATE(*tile, STATE_STATIONARY);
            
            // Set new position
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

static void explodeAt(u8 x, u8 y, u8 w, u8 h)
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
static void handleFirefly(ObjectState* tile, u8 x, u8 y, u8 w, u8 h)
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
                //explodeAt(x, y, w, h);
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

void gameUpdatePhysics()
{
    u8 w = gameState.gridWidth;   
    u8 h = gameState.gridHeight;   
    
    if (gameState.physicsWaitingForPlayer) {
        return;
    }
    
    if (gameState.physicsX == 0 && gameState.physicsY == 0) {
        for (u8 cy = 0; cy < h; cy++) {
            for (u8 cx = 0; cx < w; cx++) {
                CLEAR_SCANNED(gameState.objectGrid[cx][cy]);
            }
        }
        memcpy(&gameState.betweenFramesGrid, &gameState.visualGrid, MAX_GRID_SIZE * 2 * MAX_GRID_SIZE * 2);
    }
    
    u8 startX = gameState.physicsX;
    for (u8 y = gameState.physicsY; y < h; y++) {
        for (u8 x = startX; x < w; x++) {
            if (IS_SCANNED(gameState.objectGrid[x][y])) {
                continue;
            }            
            ObjectState* tile = &gameState.objectGrid[x][y];

            if (tile->object == TILE_PLAYER) {
                gameState.playerX = x;
                gameState.playerY = y;
                if (gameState.playerDirectionX != 0 || gameState.playerDirectionY != 0) {
                    memcpy(&gameState.betweenFramesGrid, &gameState.visualGrid, MAX_GRID_SIZE * 2 * MAX_GRID_SIZE * 2);
                    updatePlayerPhysics();
                    gameState.playerDirectionX = 0;
                    gameState.playerDirectionY = 0;
                } else {
                    gameState.physicsX = x;
                    gameState.physicsY = y;
                    gameState.playerDirectionX = 0;
                    gameState.playerDirectionY = 0;
                    gameState.physicsWaitingForPlayer = TRUE;
                    return;
                }
            }
            switch (tile->object) {
                case TILE_BOULDER:
                case TILE_DIAMOND:
                    handleRollingObjects(tile, x, y, w, h);
                    break;
                
                case TILE_FIREFLY:
                    handleFirefly(tile, x, y, w, h);
                    break;
                
                default:
                    continue;
            }
        }
        startX = 0;
    }
    
    gameState.physicsX = 0;
    gameState.physicsY = 0;
}

void gameSaveState()
{
    if (gameState.historyIndex < MAX_HISTORY) {
        memcpy(&gameState.history[gameState.historyIndex], &gameState.objectGrid, sizeof(gameState.objectGrid));
        gameState.historyIndex++;
    } else {
        for (u16 i = 1; i < MAX_HISTORY; i++) {
            memcpy(&gameState.history[i - 1], &gameState.history[i], sizeof(gameState.objectGrid));
        }
        memcpy(&gameState.history[MAX_HISTORY - 1], &gameState.objectGrid, sizeof(gameState.objectGrid));
    }
}

void gameRewindState()
{
    if (gameState.historyIndex > 1) {
        gameState.historyIndex--;
        memcpy(&gameState.objectGrid, &gameState.history[gameState.historyIndex - 1], sizeof(gameState.objectGrid));
        
        // Rebuild visualGrid from objectGrid
        for (u8 x = 0; x < gameState.gridWidth; x++) {
            for (u8 y = 0; y < gameState.gridHeight; y++) {
                setVisualMapTile(x * 2, y * 2, gameState.objectGrid[x][y].object);
            }
        }
        redrawStage = REDRAW_STAGE_BETWEEN;
    }
}

void gameRestartMap()
{
    gameState.historyIndex = 1;
    memcpy(&gameState.objectGrid, &gameState.original, sizeof(gameState.objectGrid));
    
    // Rebuild visualGrid from objectGrid
    for (u8 x = 0; x < gameState.gridWidth; x++) {
        for (u8 y = 0; y < gameState.gridHeight; y++) {
            setVisualMapTile(x * 2, y * 2, gameState.objectGrid[x][y].object);
        }
    }
    redrawStage = REDRAW_STAGE_BETWEEN;
}

