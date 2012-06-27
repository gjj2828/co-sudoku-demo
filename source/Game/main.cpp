#include "StdAfx.h"
#include "Game.h"

extern "C"
{
    DLL_EXPORT IGame* CreateGame()
    {
        static char buffer[sizeof(CGame)];
        return new ((void*)buffer) CGame();
    }
};