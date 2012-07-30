#ifndef __GRIDMANAGER_H__
#define __GRIDMANAGER_H__

#include <IGridManager.h>
#include "RenderSystem.h"

class CGridManager: public IGridManager
{
public:
    CGridManager();
    ~CGridManager();
    virtual int GetGrid(POINT pos);

    void SetFrameRGN(HRGN rgn);
    void SetGridRGN(int grid, HRGN rgn);

private:
    HRGN    m_hpFrameRGN;
    HRGN    m_hpGridRGN[CRenderSystem::GAN];
    int     m_iLastGrid;

    BOOL IsPointInRgn(HRGN rgn, POINT pt) {return PtInRegion(rgn, pt.x, pt.y);}
};

#endif // __GRIDMANAGER_H__