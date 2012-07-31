#ifndef __IRENDERSYSTEM_H__
#define __IRENDERSYSTEM_H__

#include <IGridManager.h>

class IRenderSystem
{
public:
    virtual int     Init(HWND hwnd, IGridManager*& gm)  = 0;
    virtual void    Release()                           = 0;
    virtual void    Update()                            = 0;
    virtual void    Update(int grid)                    = 0;
    virtual void    SetSelectedGrid(int grid)           = 0;
};

#endif // __IRENDERSYSTEM_H__