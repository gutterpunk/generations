/**
 * Boulder Dash Puzzle Game
 * Turn-based physics puzzle with rewind mechanics
 **/
#include <genesis.h>
#include "engine.h"
#include "game.h"
#include "map.h"
#include "player.h"

int main()
{
    engineInit();
    gameInit();
    SYS_doVBlankProcess();
    
    while(TRUE)
    {
        JOY_update();
        playerHandleInput();
        
        if (gameState.gameState == GAME_STATE_PLAYING) {
            if (frameCounter % 4 == 0) { 
                gameUpdatePhysics();
                engineRender();
            }
            frameCounter++;
        }
        
        SYS_doVBlankProcess();
    }
    
    return 0;
}

