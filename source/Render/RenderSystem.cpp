#include "StdAfx.h"
#include "RenderSystem.h"

CRenderSystem::CRenderSystem(): m_hWnd(NULL)
{
}

int CRenderSystem::Init(HWND hwnd)
{
    m_hWnd = hwnd;
    GetClientRect(m_hWnd, &m_ClientRect);

    return 1;
}

void CRenderSystem::Update()
{
    PAINTSTRUCT ps;
    POINT pts[2];
    pts[0].x = m_ClientRect.left;
    pts[0].y = m_ClientRect.top;
    pts[1].x = m_ClientRect.right;
    pts[1].y = m_ClientRect.bottom;
    HDC hdc = GetDC(m_hWnd);
    //HPEN hpen = CreatePen(PS_SOLID, 5, COL_BLACK);
    //SelectObject(hdc, hpen);
    HFONT hfont = CreateFont(30, 30, 0, 0, FW_NORMAL, false, false, false, GB2312_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_MODERN, NULL);
    SelectObject(hdc, hfont);
    SetDCPenColor(hdc, COL_BLACK);
    //RECT rect = {10, 10, 20, 20};
    //DrawText(hdc, "0", 1, &rect, 0);
    TextOut(hdc, 10, 10, "0", 1);
    //Polyline(hdc, pts, 2);
    ReleaseDC(m_hWnd, hdc);
    DeleteObject(hfont);
    //DeleteObject(hpen);
}