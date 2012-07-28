#ifndef __GRIDMANAGER_H__
#define __GRIDMANAGER_H__

#include <IGridManager.h>
#include "RenderSystem.h"

class CGridManager: public IGridManager
{
public:
    CGridManager();
    ~CGridManager();
    virtual void SetPos(POINT pos) {}
    virtual int GetSelectedGrid() {return 0;}

    void SetGridRGN(int grid, HRGN rgn);

private:
    HRGN GridRGN[CRenderSystem::GAN];
};

#endif // __GRIDMANAGER_H__