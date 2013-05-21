//**************************************************
//File: main.cpp
//Author: GaoJiongjiong
//Function: ÓÎÏ·Ö÷Ìå
//**************************************************

#include "StdAfx.h"
#include <CommonModule.h>
#include "Game.h"

extern "C"
{
    DLL_EXPORT IGame* CreateGame()
    {
        static char buffer[sizeof(CGame)];
        IGame* pGame = new ((void*)buffer) CGame();;
        ModuleInit(pGame->GetEnv());
        return pGame;
    }
};