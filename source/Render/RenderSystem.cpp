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
    HDC hdc = BeginPaint(m_hWnd, &ps);
    SetDCPenColor(hdc, COL_BLACK);
    Polyline(hdc, pts, 2);
    EndPaint(m_hWnd, &ps);
}