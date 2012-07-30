#include "StdAfx.h"
#include "GridManager.h"

CGridManager::CGridManager()
: m_hpFrameRGN(NULL)
, m_iLastGrid(INVALID_GRID)
{
    ZeroMemory(m_hpGridRGN, sizeof(HRGN) * CRenderSystem::GAN);
}

CGridManager::~CGridManager()
{
    SAFE_DELETEOBJECT(m_hpFrameRGN);
    for(int i = 0; i < CRenderSystem::GAN; i++)
    {
        SAFE_DELETEOBJECT(m_hpGridRGN[i]);
    }
}

int CGridManager::GetGrid(POINT pos)
{
    if(m_iLastGrid == INVALID_GRID)
    {
        if(!IsPointInRgn(m_hpFrameRGN, pos)) return INVALID_GRID;
    }
    else
    {
        if(IsPointInRgn(m_hpGridRGN[m_iLastGrid], pos)) return m_iLastGrid;
    }

    for(int i = 0; i < CRenderSystem::GAN; i++)
    {
        if(IsPointInRgn(m_hpGridRGN[i], pos)) return i;
    }

    return INVALID_GRID;
}

void CGridManager::SetFrameRGN(HRGN rgn)
{
    m_hpFrameRGN = rgn;
}

void CGridManager::SetGridRGN(int grid, HRGN rgn)
{
    if(grid < 0 || grid >= CRenderSystem::GAN) return;
    m_hpGridRGN[grid] = rgn;
}