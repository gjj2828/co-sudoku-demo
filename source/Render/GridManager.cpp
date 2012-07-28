#include "StdAfx.h"
#include "GridManager.h"

CGridManager::CGridManager()
{
    ZeroMemory(GridRGN, sizeof(HRGN) * CRenderSystem::GAN);
}

CGridManager::~CGridManager()
{
    for(int i = 0; i < CRenderSystem::GAN; i++)
    {
        SAFE_DELETEOBJECT(GridRGN[i]);
    }
}

void CGridManager::SetGridRGN(int grid, HRGN rgn)
{
    if(grid < 0 || grid >= CRenderSystem::GAN) return;
    GridRGN[grid] = rgn;
}