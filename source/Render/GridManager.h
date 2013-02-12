//**************************************************
//File: GridManager.cpp
//Author: GaoJiongjiong
//Function: Õ¯∏Òπ‹¿Ì
//**************************************************

#ifndef __GRIDMANAGER_H__
#define __GRIDMANAGER_H__

#include <IGridManager.h>
#include "RenderSystem.h"

class CGridManager: public IGridManager
{
public:
    CGridManager();
    ~CGridManager();
    virtual void GetGrid(POINT pos, int& grid, int& sgrid);

    void SetFrameRGN(HRGN rgn);
    void SetGridRGN(int grid, HRGN rgn);
    void SetSGridRGN(int grid, int sgrid, HRGN rgn);

private:
    HRGN    m_hpFrameRGN;
    HRGN    m_hpGridRGN[CRenderSystem::GAN];
    HRGN    m_hpSGridRGN[CRenderSystem::GAN][CRenderSystem::SGGAN];
    int     m_iLastGrid;
    int     m_iLastSGrid;

    BOOL IsPointInRgn(HRGN rgn, POINT pt) {return PtInRegion(rgn, pt.x, pt.y);}
};

#endif // __GRIDMANAGER_H__