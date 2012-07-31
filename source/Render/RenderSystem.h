#ifndef __RENDERSYSTEM_H__
#define __RENDERSYSTEM_H__

#include <IRenderSystem.h>

class CRenderSystem: public IRenderSystem
{
public:
    enum
    {
        FLW     = 2,
        NLW     = 1,
        WLW     = 2,
        SGW     = 16,
        SGGLN   = 3,
        GW      = SGW * SGGLN,
        GBGLN   = 3,
        GBGAN   = GBGLN * GBGLN,
        NLBGN   = 2,
        BGW     = GW * GBGLN + NLW * NLBGN,
        BGLN    = 3,
        GLN     = GBGLN * BGLN,
        BGAN    = BGLN * BGLN,
        GAN     = GBGAN * BGAN,
        FLN     = 2,
        NLN     = NLBGN * BGLN,
        WLN     = 2,
        W       = BGW * BGLN + FLW + FLN + WLW * WLN,
    };

    CRenderSystem();
    virtual int     Init(HWND hwnd, IGridManager*& gm);
    virtual void    Release();
    virtual void    Update();
    virtual void    Update(int ogrid);
    virtual void    SetSelectedGrid(int grid);

private:
    struct GridData
    {
        RECT    rect;
        int     num;
    };
    struct LineData
    {
        POINT pts[2];
    };

    HWND        m_hWnd;
    RECT        m_ClientRect;

    HPEN        m_hpFrame;
    HPEN        m_hpWL;
    HPEN        m_hpNL;

    HBRUSH      m_hbChoiced;
    HBRUSH      m_hbUnChoiced;

    HFONT       m_hFont;

    POINT       m_Start;
    GridData    m_Grids[GAN];
    LineData    m_NVLine[NLN];
    LineData    m_NHLine[NLN];
    LineData    m_WVLine[WLN];
    LineData    m_WHLine[WLN];

    int         m_iSelectedGrid;

    void DrawGrid(HDC hdc, int grid);
};

#endif // __RENDERSYSTEM_H__