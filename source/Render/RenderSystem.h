#ifndef __RENDERSYSTEM_H__
#define __RENDERSYSTEM_H__

#include <IRenderSystem.h>

class CRenderSystem: public IRenderSystem
{
public:
    CRenderSystem();
    virtual int     Init(HWND hwnd);
    virtual void    Release()   {this->~CRenderSystem();}
    virtual void    Update();

private:
    HWND m_hWnd;
    RECT m_ClientRect;
};

#endif // __RENDERSYSTEM_H__