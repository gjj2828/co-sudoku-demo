#include "StdAfx.h"
#include "Game.h"

#define CLASS_NAME "Sudoku"
#define WINDOW_NAME "Sudoku"

CGame::CGame()
: m_hInstance(NULL)
, m_hWnd(NULL)
, m_hBkBrush(NULL)
, m_pPuzzleSystem(NULL)
, m_pRenderSystem(NULL)
{
    ZeroMemory(m_hModules, sizeof(HMODULE) * EMODULE_MAX);
}

int CGame::Init(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    if(!InitWindow()) return 0;
    if(!LoadDll()) return 0;

    ShowWindow(m_hWnd, SW_NORMAL);
    UpdateWindow(m_hWnd);

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
    SAFE_DELETEOBJECT(m_hBkBrush);
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
    if(!m_pRenderSystem->Init(m_hWnd))    return 0;

    return 1;
}

int CGame::InitWindow()
{
    m_hBkBrush = CreateSolidBrush(COL_WHITE);
    WNDCLASS wc;
    wc.style            = 0;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = m_hInstance;
    wc.hIcon            = NULL;
    wc.hCursor          = NULL;
    wc.hbrBackground    = m_hBkBrush;
    wc.lpszMenuName     = MAKEINTRESOURCE(IDC_MENU);
    wc.lpszClassName    = CLASS_NAME;

    if(!RegisterClass(&wc)) return 0;

    m_hWnd = CreateWindow( CLASS_NAME, WINDOW_NAME, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
                         , CW_USEDEFAULT, CW_USEDEFAULT, 480, 520, NULL, LoadMenu(GetModuleHandle("Game"),  MAKEINTRESOURCE(IDC_MENU)), m_hInstance, NULL );

    if(!m_hWnd) return 0;

    return 1;
}

LRESULT CGame::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    int wmId, wmEvent;

    switch(message)
    {
    case WM_COMMAND:
        wmId = LOWORD(wparam);
        wmEvent = HIWORD(wparam);
        switch(wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        default:
            return DefWindowProc(hwnd, message, wparam, lparam);
        }
        break;
    case WM_PAINT:
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }

    return 0;
}