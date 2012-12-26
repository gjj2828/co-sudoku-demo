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
#if USE_CONSOLE
    AllocConsole();
#endif // USE_CONSOLE
    FILE* conout = NULL;
    FILE* conin = NULL;
    freopen_s(&conout, "conout$", "w", stdout);
    freopen_s(&conin, "conin$", "r", stdin);
    if(pGame->Init(hInstance))
    {
        pGame->Run();
    }
    SAFE_RELEASE(pGame);
    FreeLibrary(hGameDll);
    SAFE_FCLOSE(conout);
    SAFE_FCLOSE(conin);
#if USE_CONSOLE
    FreeConsole();
#endif // USE_CONSOLE
    return 0;
}