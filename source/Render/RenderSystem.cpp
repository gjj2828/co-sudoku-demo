#include "StdAfx.h"
#include "RenderSystem.h"

CRenderSystem::CRenderSystem()
: m_hWnd(NULL)
, m_hpFrame(NULL)
, m_hpWL(NULL)
, m_hpNL(NULL)
, m_hFont(NULL)
, m_hbChoiced(NULL)
{
    ZeroMemory(&m_ClientRect, sizeof(RECT));
}

int CRenderSystem::Init(HWND hwnd, RenderData* pdata)
{
    m_hWnd = hwnd;
    GetClientRect(m_hWnd, &m_ClientRect);

    int width = m_ClientRect.right - m_ClientRect.left;
    int hight = m_ClientRect.bottom - m_ClientRect.top;
    if(width < RenderData::W || hight < RenderData::W) ERROR_RTN0("ClientRect is too small!");

    m_hpFrame   = CreatePen(PS_INSIDEFRAME, RenderData::FLW, COL_BLACK);
    m_hpWL      = CreatePen(PS_SOLID, RenderData::WLW, COL_BLACK);
    m_hpNL      = CreatePen(PS_SOLID, RenderData::NLW, COL_BLACK);

    m_hbChoiced = CreateSolidBrush(COL_GREY);

    m_hFont     = CreateFont(RenderData::GW, 0, 0, 0, FW_NORMAL, false, false, false, GB2312_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, NULL);

    pdata->m_Start.x = m_ClientRect.left + (width - RenderData::W) / 2;
    pdata->m_Start.y = m_ClientRect.top + (hight - RenderData::W) / 2;

    for(int i = 0; i < RenderData::WLN; i++)
    {
        pdata->m_WVLine[i].pts[0].x = pdata->m_Start.x + RenderData::FLW + RenderData::BGW * (i + 1) + RenderData::WLW * i + RenderData::WLW / 2;
        pdata->m_WVLine[i].pts[0].y = pdata->m_Start.y;
        pdata->m_WVLine[i].pts[1].x = pdata->m_WVLine[i].pts[0].x;
        pdata->m_WVLine[i].pts[1].y = pdata->m_WVLine[i].pts[0].y + RenderData::W - 1;

        pdata->m_WHLine[i].pts[0].x = pdata->m_Start.x;
        pdata->m_WHLine[i].pts[0].y = pdata->m_Start.y + RenderData::FLW + RenderData::BGW * (i + 1) + RenderData::WLW * i + RenderData::WLW / 2;
        pdata->m_WHLine[i].pts[1].x = pdata->m_WHLine[i].pts[0].x + RenderData::W - 1;
        pdata->m_WHLine[i].pts[1].y = pdata->m_WHLine[i].pts[0].y;
    }

    for(int i = 0; i < RenderData::BGLN; i++)
    {
        for(int j = 0; j < RenderData::NLBGN; j++)
        {
            int index = RenderData::NLBGN * i + j;
            pdata->m_NVLine[index].pts[0].x = pdata->m_Start.x + RenderData::FLW + (RenderData::BGW + RenderData::WLW) * i + RenderData::GW * (j + 1) + RenderData::NLW * j + RenderData::NLW / 2;
            pdata->m_NVLine[index].pts[0].y = pdata->m_Start.y;
            pdata->m_NVLine[index].pts[1].x = pdata->m_NVLine[index].pts[0].x;
            pdata->m_NVLine[index].pts[1].y = pdata->m_NVLine[index].pts[0].y + RenderData::W - 1;

            pdata->m_NHLine[index].pts[0].x = pdata->m_Start.x;
            pdata->m_NHLine[index].pts[0].y = pdata->m_Start.y + RenderData::FLW + (RenderData::BGW + RenderData::WLW) * i + RenderData::GW * (j + 1) + RenderData::NLW * j + RenderData::NLW / 2;
            pdata->m_NHLine[index].pts[1].x = pdata->m_NHLine[index].pts[0].x + RenderData::W - 1;
            pdata->m_NHLine[index].pts[1].y = pdata->m_NHLine[index].pts[0].y;
        }
    }

    for(int i = 0; i < RenderData::BGLN; i++)
    {
        for(int j = 0; j < RenderData::BGLN; j++)
        {
            for(int k = 0; k < RenderData::GBGLN; k++)
            {
                for(int l = 0; l < RenderData::GBGLN; l++)
                {
                    int index = RenderData::GLN * (RenderData::GBGLN * i + k) + RenderData::GBGLN * j + l;
                    //int row     = GBGLN * i + k;
                    //int column  = GBGLN * j + l;
                    //int index   = GLN * row + column;

                    pdata->m_Grids[index].rect.top     = pdata->m_Start.y + RenderData::FLW + (RenderData::BGW + RenderData::WLW) * i + (RenderData::GW + RenderData::NLW) * k;
                    pdata->m_Grids[index].rect.left    = pdata->m_Start.x + RenderData::FLW + (RenderData::BGW + RenderData::WLW) * j + (RenderData::GW + RenderData::NLW) * l;
                    pdata->m_Grids[index].rect.bottom  = pdata->m_Grids[index].rect.top + RenderData::GW;
                    pdata->m_Grids[index].rect.right   = pdata->m_Grids[index].rect.left + RenderData::GW;

                    pdata->m_Grids[index].hrgn = CreateRectRgnIndirect(&(pdata->m_Grids[index].rect));
                }
            }
        }
    }

    return 1;
}

void CRenderSystem::Release()
{
    SAFE_DELETEOBJECT(m_hpFrame);
    SAFE_DELETEOBJECT(m_hpWL);
    SAFE_DELETEOBJECT(m_hpNL);
    SAFE_DELETEOBJECT(m_hbChoiced);
    SAFE_DELETEOBJECT(m_hFont);
    this->~CRenderSystem();
}

void CRenderSystem::Update(RenderData* pdata)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);
    SelectObject(hdc, m_hpFrame);
    Rectangle(hdc, pdata->m_Start.x, pdata->m_Start.y, pdata->m_Start.x + RenderData::W, pdata->m_Start.y + RenderData::W);
    SelectObject(hdc, m_hpWL);
    for(int i = 0; i < RenderData::WLN; i++)
    {
        Polyline(hdc, pdata->m_WVLine[i].pts, 2);
        Polyline(hdc, pdata->m_WHLine[i].pts, 2);
    }
    SelectObject(hdc, m_hpNL);
    for(int i = 0; i < RenderData::NLN; i++)
    {
        Polyline(hdc, pdata->m_NVLine[i].pts, 2);
        Polyline(hdc, pdata->m_NHLine[i].pts, 2);
    }
    SelectObject(hdc, m_hFont);
    SetBkMode(hdc, TRANSPARENT);
    for(int i = 0; i < RenderData::GAN; i++)
    {
        if(i % 2 == 0) FillRect(hdc, &(pdata->m_Grids[i].rect), m_hbChoiced);
        char num = '0' + pdata->m_Grids[i].num;
        DrawText(hdc, &num, 1, &(pdata->m_Grids[i].rect), DT_CENTER | DT_VCENTER);
    }
    EndPaint(m_hWnd, &ps);
}