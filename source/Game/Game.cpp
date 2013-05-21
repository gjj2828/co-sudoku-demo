//**************************************************
//File: Game.cpp
//Author: GaoJiongjiong
//Function: ÓÎÏ·Ö÷Ìå
//**************************************************

#include "StdAfx.h"
#include "Game.h"
#include <IPuzzleSystem.h>
#include <IRenderSystem.h>
#include "Timer.h"

const char* CGame::m_cClassName = GAME_NAME;
const char* CGame::m_cWindowName = GAME_NAME;

const float CGame::m_cSPF = 1.0f / (float)CGame::FPS;

CGame::CGame()
: m_hInstance(NULL)
, m_hWnd(NULL)
, m_hBkBrush(NULL)
, m_pGridManager(NULL)
, m_iSelectedGrid(IGridManager::INVALID_GRID)
, m_pTimer(NULL)
, m_fTime(0)
, m_eCoType(ECOTYPE_MIN)
, m_eATState(EATSTATE_MIN)
{
    ZeroMemory(m_hModules, sizeof(HMODULE) * EMODULE_MAX);
    ZeroMemory(&m_env, sizeof(GlobalEnviroment));
    m_env.pGame = this;
}

int CGame::Init(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    if(!InitWindow())   return 0;
    if(!LoadDll())      return 0;
    if(!InitTimer())    return 0;

    //ShowWindow(m_hWnd, SW_NORMAL);
    //UpdateWindow(m_hWnd);

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

        Update();
    }
}

void    CGame::Release()
{
    SAFE_DELETEOBJECT(m_hBkBrush);
    SAFE_RELEASE(m_env.pPuzzleSystem);
    SAFE_RELEASE(m_env.pRenderSystem);
    SAFE_DELETE(m_pGridManager);
    SAFE_DELETE(m_pTimer);
    for(int i = EMODULE_MIN; i < EMODULE_MAX; i++)
    {
        SAFE_FREELIBRARY(m_hModules[i]);
    }
    UnregisterClass(m_cClassName, m_hInstance);
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

    // EMODULE_RENDER
    hModule = LoadLibrary("Network.dll");
    if(!hModule) ERROR_RTN0("Can\'t load Network.dll!");
    m_hModules[EMODULE_NETWORK] = hModule;
    typedef INetworkSystem* (*CreateNetworkSystemFunc)();
    CreateNetworkSystemFunc fCreateNetworkSystem = (CreateNetworkSystemFunc)GetProcAddress(hModule, "CreateNetworkSystem");
    if(!fCreateNetworkSystem) ERROR_RTN0("Can\'t get CreateNetworkSystem function!");
    m_env.pNetworkSystem = fCreateNetworkSystem();
    if(!m_env.pNetworkSystem) ERROR_RTN0("CreateNetworkSystem failed!");
    if(!m_env.pNetworkSystem->Init()) return 0;

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
    wc.lpszClassName    = m_cClassName;

    if(!RegisterClass(&wc)) return 0;

    m_hWnd = CreateWindow( m_cClassName, m_cWindowName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
                         , CW_USEDEFAULT, CW_USEDEFAULT, 480, 512, NULL, LoadMenu(GetModuleHandle("Game"),  MAKEINTRESOURCE(IDC_MENU)), m_hInstance, NULL );

    if(!m_hWnd) return 0;

    return 1;
}

void CGame::Update()
{
    UpdateTimer();
    m_env.pNetworkSystem->Update(m_fTime);

    static bool s_bFlag = false;
    static float s_fWait = 0.0f;
    if(!s_bFlag && m_eCoType == ECOTYPE_SINGLE && m_fTime > 3.0f)
    {
        s_bFlag = true;
        if(m_env.pNetworkSystem->Start(INetworkSystem::EMODE_AUTOPAIR, m_fTime))
        {
            m_eCoType = ECOTYPE_AUTOPAIR;
            m_eATState = EATSTATE_WAITING;
            m_env.pNetworkSystem->RegisterListener(this);
            srand(GetTickCount());
            s_fWait = m_fTime + rand() * 5.0f / RAND_MAX;
        }
        else
        {
            PRINT("NetworkSystem Start Error!\n");
        }
    }
    if(m_eCoType == ECOTYPE_AUTOPAIR && (m_eATState == EATSTATE_HOST || m_eATState == EATSTATE_CLIENT))
    {
        if(m_fTime > s_fWait)
        {
            Test1Packet packet;
            packet.val = m_fTime;
            m_env.pNetworkSystem->Send(&packet);
            PRINT("SendTest1: %f\n", packet.val);
            s_fWait = m_fTime + rand() * 5.0f / RAND_MAX;
        }
    }
}

int CGame::InitTimer()
{
    m_pTimer = new CPrecisionTimer;
    if(m_pTimer->Init()) return 1;
    SAFE_DELETE(m_pTimer);
    m_pTimer = new CNormalTimer;
    if(!m_pTimer->Init()) ERROR_RTN0("Can\'t init timer!");
    float time = m_pTimer->GetTime();

    return 1;
}

void CGame::UpdateTimer()
{
    float dt = m_pTimer->GetTime() - m_fTime;
    if(dt < m_cSPF) Sleep((DWORD)((m_cSPF - dt) * 1000.0f));

    m_fTime = m_pTimer->GetTime();
}

void CGame::OnAccept(int client)
{
    PRINT("OnAccept client: %d\n", client);

    if(m_eATState == EATSTATE_WAITING) m_eATState = EATSTATE_HOST;
    Test2Packet packet;
    packet.val = m_fTime;
    m_env.pNetworkSystem->Send(&packet, client);
    PRINT("SendTest2: %f\n", packet.val);
}

void CGame::OnConnect()
{
    PRINT("OnConnect\n");

    if(m_eATState == EATSTATE_WAITING) m_eATState = EATSTATE_CLIENT;
}

void CGame::OnRecv(Packet* pPacket)
{
    switch(pPacket->type)
    {
    case EPACKETTYPE_TEST1:
        PRINT("RecvTest1: %f\n", ((Test1Packet*)pPacket)->val);
        break;
    case EPACKETTYPE_TEST2:
        PRINT("RecvTest2: %f\n", ((Test2Packet*)pPacket)->val);
        break;
    default:
        PRINT("Recv Unknown packet!\n");
        break;
    }
}

void CGame::OnDisconnect()
{
    PRINT("OnDisconnect\n");

    m_eATState = EATSTATE_WAITING;
}

void CGame::OnStop()
{
    PRINT("OnStop\n");

    m_eCoType = ECOTYPE_SINGLE;
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