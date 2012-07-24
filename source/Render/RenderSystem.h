#ifndef __RENDERSYSTEM_H__
#define __RENDERSYSTEM_H__

#include <IRenderSystem.h>

class CRenderSystem: public IRenderSystem
{
public:
    CRenderSystem();
    virtual int     Init(HWND hwnd, RenderData* pdata);
    virtual void    Release();
    virtual void    Update(RenderData* pdata);

private:
    HWND        m_hWnd;
    RECT        m_ClientRect;

    HPEN        m_hpFrame;
    HPEN        m_hpWL;
    HPEN        m_hpNL;

    HBRUSH      m_hbChoiced;

    HFONT       m_hFont;

};

#endif // __RENDERSYSTEM_H__