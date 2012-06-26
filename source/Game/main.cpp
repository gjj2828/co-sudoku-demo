#include "StdAfx.h"
#include "Game.h"

extern "C"
{
    GAME_API IGame* CreateGame()
    {
        static char buffer[sizeof(CGame)];
        return new ((void*)buffer) CGame();
    }
};