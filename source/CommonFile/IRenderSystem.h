#ifndef __IRENDERSYSTEM_H__
#define __IRENDERSYSTEM_H__

#include <RenderData.h>

class IRenderSystem
{
public:
    virtual int     Init(HWND hwnd, RenderData* pdata)  = 0;
    virtual void    Release()                           = 0;
    virtual void    Update(RenderData* pdata)           = 0;
};

#endif // __IRENDERSYSTEM_H__