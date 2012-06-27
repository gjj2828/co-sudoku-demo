#include "StdAfx.h"
#include <IGame.h>

int APIENTRY    WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    InitRootDir();
    HMODULE hGameDll = LoadLibrary("Game.dll");
    if(!hGameDll)   ERROR_EXIT("Can\'t load Game.dll!");
    typedef IGame*  (*CreateGameFunc)();
    CreateGameFunc  fCreateGame = (CreateGameFunc)GetProcAddress(hGameDll, "CreateGame");
    if(!fCreateGame)
    {
        FreeLibrary(hGameDll);
        ERROR_EXIT("Can\'t get CreateGame function!");
    }
    IGame*  pGame   = fCreateGame();
    if(!pGame)
    {
        FreeLibrary(hGameDll);
        ERROR_EXIT("CreateGame failed!");
    }
    if(pGame->Init())
    {
        pGame->Run();
    }
    pGame->Release();
    FreeLibrary(hGameDll);
    return 0;
}