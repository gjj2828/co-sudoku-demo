#ifndef __GRIDMANAGER_H__
#define __GRIDMANAGER_H__

#include <IGridManager.h>
#include "RenderSystem.h"

class CGridManager: public IGridManager
{
public:
    virtual void SetPos(POINT pos) {}
    virtual int GetSelectedGrid() {return 0;}

private:
};

#endif // __GRIDMANAGER_H__