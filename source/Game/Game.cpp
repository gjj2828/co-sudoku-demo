#include "StdAfx.h"
#include "Game.h"

#define CLASS_NAME "Sudoku"
#define WINDOW_NAME "Sudoku"

CGame::CGame()
: m_hInstance(NULL)
, m_hWnd(NULL)
, m_hBkBrush(NULL)
, m_pGridManager(NULL)
, m_iSelectedGrid(IGridManager::INVALID_GRID)
{
    ZeroMemory(m_hModules, sizeof(HMODULE) * EMODULE_MAX);
    ZeroMemory(&m_env, sizeof(GlobalEnviroment));
    m_env.pGame = this;
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
    while(1)
    {
        MSG Msg;
        bool bQuit = false;

        while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            if(Msg.message == WM_QUIT)
            {
                bQuit = true;
                break;
            }

            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }

        if(bQuit) break;
    }
}

void    CGame::Release()
{
    SAFE_DELETEOBJECT(m_hBkBrush);
    SAFE_RELEASE(m_env.pPuzzleSystem);
    SAFE_RELEASE(m_env.pRenderSystem);
    SAFE_DELETE(m_pGridManager);
    for(int i = EMODULE_MIN; i < EMODULE_MAX; i++)
    {
        SAFE_FREELIBRARY(m_hModules[i]);
    }
    UnregisterClass(CLASS_NAME, m_hInstance);
    m_env.pGame = NULL;
    this->~CGame();
}

void CGame::Paint()
{
    POINT pos;
    GetCursorPos(&pos);
    ScreenToClient(m_hWnd, &pos);

    m_iSelectedGrid = m_pGridManager->GetGrid(pos);

    m_env.pRenderSystem->SetSelectedGrid(m_iSelectedGrid);
    m_env.pRenderSystem->Update();
}

void CGame::MouseMove(int x, int y)
{
    POINT pos = {x, y};

    int grid = m_pGridManager->GetGrid(pos);
    if(grid == m_iSelectedGrid) return;

    m_env.pRenderSystem->SetSelectedGrid(grid);
    m_env.pRenderSystem->Update(m_iSelectedGrid);
    m_env.pRenderSystem->Update(grid);

    m_iSelectedGrid = grid;
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
    m_env.pPuzzleSystem = fCreatePuzzleSystem();
    if(!m_env.pPuzzleSystem)    ERROR_RTN0("CreatePuzzleSystem failed!");
    if(!m_env.pPuzzleSystem->Init())    return 0;

    // EMODULE_RENDER
    hModule = LoadLibrary("Render.dll");
    if(!hModule)    ERROR_RTN0("Can\'t load Render.dll!");
    m_hModules[EMODULE_RENDER]  = hModule;
    typedef IRenderSystem*  (*CreateRenderSystemFunc)();
    CreateRenderSystemFunc  fCreateRenderSystem = (CreateRenderSystemFunc)GetProcAddress(hModule, "CreateRenderSystem");
    if(!fCreateRenderSystem)    ERROR_RTN0("Can\'t get CreateRenderSystem function!");
    m_env.pRenderSystem = fCreateRenderSystem();
    if(!m_env.pRenderSystem)    ERROR_RTN0("CreateRenderSystem failed!");
    if(!m_env.pRenderSystem->Init(m_hWnd, m_pGridManager))    return 0;
    if(!m_pGridManager) ERROR_RTN0("Can\'t, create GridManager!");

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
                         , CW_USEDEFAULT, CW_USEDEFAULT, 480, 512, NULL, LoadMenu(GetModuleHandle("Game"),  MAKEINTRESOURCE(IDC_MENU)), m_hInstance, NULL );

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
        gEnv->pGame->Paint();
        break;
    case WM_MOUSEMOVE:
        gEnv->pGame->MouseMove(LOWORD(lparam), HIWORD(lparam));
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }

    return 0;
}