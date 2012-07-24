#ifndef __RENDERDATA_H__
#define __RENDERDATA_H__

struct RenderData
{
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

    struct GridData
    {
        RECT    rect;
        HRGN    hrgn;
        int     num;
    };
    struct LineData
    {
        POINT pts[2];
    };

    POINT       m_Start;
    GridData    m_Grids[GAN];
    LineData    m_NVLine[NLN];
    LineData    m_NHLine[NLN];
    LineData    m_WVLine[WLN];
    LineData    m_WHLine[WLN];
};

#endif // __RENDERDATA_H__