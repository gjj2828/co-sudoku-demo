#include "StdAfx.h"
#include "RenderSystem.h"
#include "GridManager.h"

CRenderSystem::CRenderSystem()
: m_hWnd(NULL)
, m_hpFrame(NULL)
, m_hpWL(NULL)
, m_hpNL(NULL)
, m_hFont(NULL)
, m_hbChoiced(NULL)
{
    memset(&m_ClientRect, 0, sizeof(RECT));
    memset(&m_Start, 0, sizeof(POINT));
    memset(m_Grids, 0, sizeof(GridData));
    memset(m_NVLine, 0, sizeof(RECT));
    memset(m_NHLine, 0, sizeof(RECT));
    memset(m_WVLine, 0, sizeof(RECT));
    memset(m_WHLine, 0, sizeof(RECT));
}

int CRenderSystem::Init(HWND hwnd, IGridManager*& gm)
{
    m_hWnd = hwnd;
    GetClientRect(m_hWnd, &m_ClientRect);

    int width = m_ClientRect.right - m_ClientRect.left;
    int hight = m_ClientRect.bottom - m_ClientRect.top;
    if(width < W || hight < W) ERROR_RTN0("ClientRect is too small!");

    m_hpFrame       = CreatePen(PS_INSIDEFRAME, FLW, COL_BLACK);
    m_hpWL          = CreatePen(PS_SOLID, WLW, COL_BLACK);
    m_hpNL          = CreatePen(PS_SOLID, NLW, COL_BLACK);

    m_hbChoiced     = CreateSolidBrush(COL_GREY);
    m_hbUnChoiced   = CreateSolidBrush(COL_WHITE);

    m_hFont         = CreateFont(GW, 0, 0, 0, FW_NORMAL, false, false, false, GB2312_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, NULL);

    m_Start.x = m_ClientRect.left + (width - W) / 2;
    m_Start.y = m_ClientRect.top + (hight - W) / 2;

    for(int i = 0; i < WLN; i++)
    {
        m_WVLine[i].pts[0].x = m_Start.x + FLW + BGW * (i + 1) + WLW * i + WLW / 2;
        m_WVLine[i].pts[0].y = m_Start.y;
        m_WVLine[i].pts[1].x = m_WVLine[i].pts[0].x;
        m_WVLine[i].pts[1].y = m_WVLine[i].pts[0].y + W - 1;

        m_WHLine[i].pts[0].x = m_Start.x;
        m_WHLine[i].pts[0].y = m_Start.y + FLW + BGW * (i + 1) + WLW * i + WLW / 2;
        m_WHLine[i].pts[1].x = m_WHLine[i].pts[0].x + W - 1;
        m_WHLine[i].pts[1].y = m_WHLine[i].pts[0].y;
    }

    for(int i = 0; i < BGLN; i++)
    {
        for(int j = 0; j < NLBGN; j++)
        {
            int index = NLBGN * i + j;
            m_NVLine[index].pts[0].x = m_Start.x + FLW + (BGW + WLW) * i + GW * (j + 1) + NLW * j + NLW / 2;
            m_NVLine[index].pts[0].y = m_Start.y;
            m_NVLine[index].pts[1].x = m_NVLine[index].pts[0].x;
            m_NVLine[index].pts[1].y = m_NVLine[index].pts[0].y + W - 1;

            m_NHLine[index].pts[0].x = m_Start.x;
            m_NHLine[index].pts[0].y = m_Start.y + FLW + (BGW + WLW) * i + GW * (j + 1) + NLW * j + NLW / 2;
            m_NHLine[index].pts[1].x = m_NHLine[index].pts[0].x + W - 1;
            m_NHLine[index].pts[1].y = m_NHLine[index].pts[0].y;
        }
    }

    CGridManager* pGridManager = new CGridManager;

    RECT FrameRect = {m_Start.x, m_Start.y, m_Start.x + W, m_Start.y + W};
    pGridManager->SetFrameRGN(CreateRectRgnIndirect(&FrameRect));

    for(int i = 0; i < BGLN; i++)
    {
        for(int j = 0; j < BGLN; j++)
        {
            for(int k = 0; k < GBGLN; k++)
            {
                for(int l = 0; l < GBGLN; l++)
                {
                    int index = GLN * (GBGLN * i + k) + GBGLN * j + l;
                    //int row     = GBGLN * i + k;
                    //int column  = GBGLN * j + l;
                    //int index   = GLN * row + column;

                    m_Grids[index].rect.top     = m_Start.y + FLW + (BGW + WLW) * i + (GW + NLW) * k;
                    m_Grids[index].rect.left    = m_Start.x + FLW + (BGW + WLW) * j + (GW + NLW) * l;
                    m_Grids[index].rect.bottom  = m_Grids[index].rect.top + GW;
                    m_Grids[index].rect.right   = m_Grids[index].rect.left + GW;

                    //m_Grids[index].hrgn = CreateRectRgnIndirect(&(m_Grids[index].rect));
                    pGridManager->SetGridRGN(index, CreateRectRgnIndirect(&(m_Grids[index].rect)));
                }
            }
        }
    }

    gm = pGridManager;

    return 1;
}

void CRenderSystem::Release()
{
    SAFE_DELETEOBJECT(m_hpFrame);
    SAFE_DELETEOBJECT(m_hpWL);
    SAFE_DELETEOBJECT(m_hpNL);
    SAFE_DELETEOBJECT(m_hbChoiced);
    SAFE_DELETEOBJECT(m_hbUnChoiced);
    SAFE_DELETEOBJECT(m_hFont);

    this->~CRenderSystem();
}

void CRenderSystem::Update()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);
    SelectObject(hdc, m_hpFrame);
    Rectangle(hdc, m_Start.x, m_Start.y, m_Start.x + W, m_Start.y + W);
    SelectObject(hdc, m_hpWL);
    for(int i = 0; i < WLN; i++)
    {
        Polyline(hdc, m_WVLine[i].pts, 2);
        Polyline(hdc, m_WHLine[i].pts, 2);
    }
    SelectObject(hdc, m_hpNL);
    for(int i = 0; i < NLN; i++)
    {
        Polyline(hdc, m_NVLine[i].pts, 2);
        Polyline(hdc, m_NHLine[i].pts, 2);
    }
    for(int i = 0; i < GAN; i++)
    {
        DrawGrid(hdc, i);
    }
    EndPaint(m_hWnd, &ps);
}

void CRenderSystem::Update(int grid)
{
    HDC hdc = GetDC(m_hWnd);
    DrawGrid(hdc, grid);
    ReleaseDC(m_hWnd, hdc);
}

void CRenderSystem::SetSelectedGrid(int grid)
{
    m_iSelectedGrid = grid;
}

void CRenderSystem::DrawGrid(HDC hdc, int grid)
{
    if(grid < 0 || grid >= IGridManager::INVALID_GRID) return;

    HBRUSH hpBrush;
    if(grid == m_iSelectedGrid) hpBrush = m_hbChoiced;
    else hpBrush = m_hbUnChoiced;

    SetBkMode(hdc, TRANSPARENT);
    FillRect(hdc, &(m_Grids[grid].rect), hpBrush);
    SelectObject(hdc, m_hFont);
    char num = '0' + m_Grids[grid].num;
    DrawText(hdc, &num, 1, &(m_Grids[grid].rect), DT_CENTER | DT_VCENTER);
}