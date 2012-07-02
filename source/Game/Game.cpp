#include "StdAfx.h"
#include "Game.h"

CGame::CGame() : m_pPuzzleSystem(NULL)
{
    ZeroMemory(m_hModules, sizeof(HMODULE) * EMODULE_MAX);
}

int CGame::Init()
{
    HMODULE         hModule;

    // EMODULE_PUZZLE
    hModule = LoadLibrary("Puzzle.dll");
    if(!hModule)    ERROR_RTN0("Can\'t load Puzzle.dll!");
    m_hModules[EMODULE_PUZZLE]  = hModule;
    typedef IPuzzleSystem*  (*CreatePuzzleSystemFunc)();
    CreatePuzzleSystemFunc  fCreatePuzzleSystem = (CreatePuzzleSystemFunc)GetProcAddress(hModule, "CreatePuzzleSystem");
    if(!fCreatePuzzleSystem)    ERROR_RTN0("Can\'t get CreatePuzzleSystem function!");
    m_pPuzzleSystem = fCreatePuzzleSystem();
    if(!m_pPuzzleSystem)    ERROR_RTN0("CreatePuzzleSystem failed!");
    if(!m_pPuzzleSystem->Init())    return 0;

    // EMODULE_RENDER
    hModule = LoadLibrary("Render.dll");
    if(!hModule)    ERROR_RTN0("Can\'t load Render.dll!");
    m_hModules[EMODULE_RENDER]  = hModule;
    typedef IRenderSystem*  (*CreateRenderSystemFunc)();
    CreateRenderSystemFunc  fCreateRenderSystem = (CreateRenderSystemFunc)GetProcAddress(hModule, "CreateRenderSystem");
    if(!fCreateRenderSystem)    ERROR_RTN0("Can\'t get CreateRenderSystem function!");
    m_pRenderSystem = fCreateRenderSystem();
    if(!m_pRenderSystem)    ERROR_RTN0("CreateRenderSystem failed!");
    if(!m_pRenderSystem->Init())    return 0;

    return 1;
}

void    CGame::Release()
{
    SAFE_RELEASE(m_pPuzzleSystem);
    SAFE_RELEASE(m_pRenderSystem);
    for(int i = EMODULE_MIN; i < EMODULE_MAX; i++)
    {
        SAFE_FREELIBRARY(m_hModules[i]);
    }
    this->~CGame();
}