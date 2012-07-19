#include "StdAfx.h"
#include "RenderSystem.h"

CRenderSystem::CRenderSystem(): m_hWnd(NULL)
{
}

int CRenderSystem::Init(HWND hwnd)
{
    m_hWnd = hwnd;
    GetClientRect(m_hWnd, &m_ClientRect);

    int width = m_ClientRect.right - m_ClientRect.left;
    int hight = m_ClientRect.bottom - m_ClientRect.top;
    if(width < W || hight < W) ERROR_RTN0("ClientRect is too small!");

    POINT start;
    start.x = m_ClientRect.left + (width - W) / 2;
    start.y = m_ClientRect.top + (hight - W) / 2;

    m_WVLine[0].pts[0] = start;
    for(int i = 0; i < WLN; i++)
    {
        if(i > 0)
        {
            m_WVLine[i]              = m_WVLine[i - 1];
            m_WVLine[i].pts[0].x    += (WLW + BGW);
            m_WVLine[i].pts[1].x     = m_WVLine[i].pts[0].x;

            m_WHLine[i]              = m_WHLine[i - 1];
            m_WHLine[i].pts[0].y    += (WLW + BGW);
            m_WHLine[i].pts[1].y     = m_WHLine[i].pts[0].y;
        }
        else
        {
            //m_WVLine[i].pts[0].x    = start.x + WLW - 1;
            m_WVLine[i].pts[0].x    = start.x;
            m_WVLine[i].pts[0].y    = start.y - 1;
            m_WVLine[i].pts[1].x    = m_WVLine[i].pts[0].x;
            m_WVLine[i].pts[1].y    = start.y + W - 1;

            m_WHLine[i].pts[0].x    = start.x - 1;
            //m_WHLine[i].pts[0].y    = start.y + WLW - 1;
            m_WHLine[i].pts[0].y    = start.y;
            m_WHLine[i].pts[1].x    = start.x + W - 1;
            m_WHLine[i].pts[1].y    = m_WHLine[i].pts[0].y;
        }
    }



    return 1;
}

void CRenderSystem::Update()
{
    /*
    POINT pts[2];
    pts[0].x = m_ClientRect.left;
    pts[0].y = m_ClientRect.top;
    pts[1].x = m_ClientRect.right;
    pts[1].y = m_ClientRect.bottom;
    //HDC hdc = GetDC(m_hWnd);
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);
    //HPEN hpen = CreatePen(PS_SOLID, 5, COL_BLACK);
    //SelectObject(hdc, hpen);
    HFONT hfont = CreateFont(32, 0, 0, 0, FW_NORMAL, false, false, false, GB2312_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, NULL);
    SelectObject(hdc, hfont);
    SetDCPenColor(hdc, COL_BLACK);
    RECT rect = {10, 10, 42, 42};
    //DrawText(hdc, "0", 1, &rect, DT_CENTER | DT_VCENTER);
    DrawText(hdc, "0", 1, &rect, DT_CENTER | DT_VCENTER);
    HBRUSH hbrush = CreateSolidBrush(COL_BLACK);
    FrameRect(hdc, &rect, hbrush);
    //Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    //char* text = "≤‚ ‘";
    //TextOut(hdc, 10, 10, text, strlen(text));
    //Polyline(hdc, pts, 2);
    //ReleaseDC(m_hWnd, hdc);
    EndPaint(m_hWnd, &ps);
    DeleteObject(hfont);
    DeleteObject(hbrush);
    //DeleteObject(hpen);
    */

    HPEN hpen = CreatePen(PS_SOLID, 2, COL_BLACK);
    HDC hdc = GetDC(m_hWnd);
    SelectObject(hdc, hpen);
    for(int i = 0; i < WLN; i++)
    {
        Polyline(hdc, m_WVLine[i].pts, 2);
        Polyline(hdc, m_WHLine[i].pts, 2);
    }
    ReleaseDC(m_hWnd, hdc);
}