#include "StdAfx.h"
#include "Game.h"

#define CLASS_NAME "Sudoku"
#define WINDOW_NAME "Sudoku"

CGame::CGame() : m_pPuzzleSystem(NULL)
{
    ZeroMemory(m_hModules, sizeof(HMODULE) * EMODULE_MAX);
}

int CGame::Init(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    if(!LoadDll()) return 0;
    if(!InitWindow()) return 0;

    return 1;
}

void CGame::Run()
{
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void    CGame::Release()
{
    SAFE_RELEASE(m_pPuzzleSystem);
    SAFE_RELEASE(m_pRenderSystem);
    for(int i = EMODULE_MIN; i < EMODULE_MAX; i++)
    {
        SAFE_FREELIBRARY(m_hModules[i]);
    }
    UnregisterClass(CLASS_NAME, m_hInstance);
    this->~CGame();
}

int CGame::LoadDll()
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

int CGame::InitWindow()
{
    /*
    WNDCLASS wc;
    wc.style            = 0;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = m_hInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = NULL;
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = MAKEINTRESOURCE(IDC_MENU);
    wc.lpszClassName    = CLASS_NAME;
    */
    WNDCLASSEX wc;
    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = m_hInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = NULL;
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName     = MAKEINTRESOURCE(IDC_MENU);
    wc.lpszClassName    = CLASS_NAME;
    wc.hIconSm          = NULL;

    if(!RegisterClassEx(&wc)) return 0;

    //m_hWnd = CreateWindow( CLASS_NAME, WINDOW_NAME, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
    //                     , CW_USEDEFAULT, CW_USEDEFAULT, 480, 500, NULL, NULL, m_hInstance, NULL );

    m_hWnd = CreateWindow( CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW
        , CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, m_hInstance, NULL );

    if(!m_hWnd) return 0;
    ShowWindow(m_hWnd, SW_NORMAL);
    UpdateWindow(m_hWnd);

    return 1;
}

LRESULT CGame::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}