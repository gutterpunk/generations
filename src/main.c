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
        gameUpdatePhysics();
        
        engineRender();
        
        SYS_doVBlankProcess();
    }
    
    return 0;
}

