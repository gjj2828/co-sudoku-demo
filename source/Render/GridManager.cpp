//**************************************************
//File: GridManager.cpp
//Author: GaoJiongjiong
//Function: Õ¯∏Òπ‹¿Ì
//**************************************************

#include "StdAfx.h"
#include "GridManager.h"

CGridManager::CGridManager()
: m_hpFrameRGN(NULL)
, m_iLastGrid(INVALID_GRID)
, m_iLastSGrid(INVALID_GRID)
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

void CGridManager::GetGrid(POINT pos, int& grid, int& sgrid)
{
    grid = m_iLastGrid;
    sgrid = m_iLastSGrid;

    if(m_iLastGrid == INVALID_GRID || !IsPointInRgn(m_hpGridRGN[m_iLastGrid], pos))
    {
        grid = INVALID_GRID;
        if(IsPointInRgn(m_hpFrameRGN, pos))
        {
            for(int i = 0; i < CRenderSystem::GAN; i++)
            {
                if(IsPointInRgn(m_hpGridRGN[i], pos))
                {
                    grid = i;
                    break;
                }
            }
        }
    }

    if(grid == INVALID_GRID)
    {
        sgrid = INVALID_GRID;
    }
    else if(grid != m_iLastGrid || !IsPointInRgn(m_hpSGridRGN[m_iLastGrid][m_iLastSGrid], pos))
    {
        sgrid = INVALID_GRID;
        for(int i = 0; i < CRenderSystem::SGGAN; i++)
        {
            if(IsPointInRgn(m_hpSGridRGN[grid][i], pos))
            {
                sgrid = i;
                break;
            }
        }
    }

    m_iLastGrid = grid;
    m_iLastSGrid = sgrid;
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

void CGridManager::SetSGridRGN(int grid, int sgrid, HRGN rgn)
{
    if(grid < 0 || grid >= CRenderSystem::GAN) return;
    if(sgrid < 0 || sgrid >= CRenderSystem::SGGAN) return;
    m_hpSGridRGN[grid][sgrid] = rgn;
}