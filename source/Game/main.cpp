#include "StdAfx.h"
#include <CommonModule.h>
#include "Game.h"

extern "C"
{
    DLL_EXPORT IGame* CreateGame()
    {
        IGame* pGame = new CGame;
        ModuleInit(pGame->GetEnv());
        return pGame;
    }
};