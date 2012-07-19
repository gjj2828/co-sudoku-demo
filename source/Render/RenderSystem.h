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
    enum
    {
        SGW     = 16,
        SGGLN   = 3,
        GW      = SGW * SGGLN,
        GBGLN   = 3,
        GBGAN   = GBGLN * GBGLN,
        BGLN    = 3,
        BGAN    = BGLN * BGLN,
        GAN     = GBGAN * BGAN,
        NLW     = 1,
        WLW     = 2,
        NLBGN   = 2,
        NLN     = NLBGN * BGLN,
        WLN     = 4,
        BGW     = GW * GBGLN + NLW * NLBGN,
        W       = BGW * BGLN + WLW * WLN,
    };

    struct GridData
    {
        RECT rect;
        int num;
    };
    struct LineData
    {
        POINT pts[2];
    };

    HWND m_hWnd;
    RECT m_ClientRect;

    GridData m_Grids[GAN];
    LineData m_NVLine[NLN];
    LineData m_NHLine[NLN];
    LineData m_WVLine[WLN];
    LineData m_WHLine[WLN];
};

#endif // __RENDERSYSTEM_H__