#ifndef __GAME_H__
#define __GAME_H__

#include <IGame.h>
#include <IGridManager.h>
#include <INetworkSystem.h>
#include "ITimer.h"

class CGame: public IGame, public INetworkListener
{
public:
    CGame();
    virtual int                 Init(HINSTANCE hInstance);
    virtual void                Run();
    virtual void                Release();
    virtual GlobalEnviroment*   GetEnv() {return &m_env;}
    virtual void                Paint();
    virtual void                MouseMove(int x, int y);

private:
    enum EModule
    {
        EMODULE_MIN,
        EMODULE_PUZZLE  = EMODULE_MIN,
        EMODULE_RENDER,
        EMODULE_NETWORK,
        EMODULE_MAX,
    };

    enum ECoType
    {
        ECOTYPE_MIN,
        ECOTYPE_SINGLE = ECOTYPE_MIN,
        ECOTYPE_AUTOPAIR,
        ECOTYPE_MAX,
    };

    enum EATState
    {
        EATSTATE_MIN,
        EATSTATE_WAITING,
        EATSTATE_HOST,
        EATSTATE_CLIENT,
    };

    enum
    {
        FPS = 60,
    };

    enum EPacketType
    {
        EPACKETTYPE_MIN,
        EPACKETTYPE_TEST = EPACKETTYPE_MIN,
        EPACKETTYPE_MAX,
    };

    struct TestPacket : public Packet
    {
        TestPacket()
        {
            size = sizeof(*this);
            type = EPACKETTYPE_TEST;
        }

        int test;
    };

    HINSTANCE           m_hInstance;
    HWND                m_hWnd;
    HBRUSH              m_hBkBrush;
    HMODULE             m_hModules[EMODULE_MAX];
    GlobalEnviroment    m_env;
    IGridManager*       m_pGridManager;
    int                 m_iSelectedGrid;

    ITimer*             m_pTimer;
    float               m_fTime;

    ECoType             m_eCoType;
    EATState            m_eATState;

    int     LoadDll();
    int     InitWindow();
    int     InitTimer();
    void    UpdateTimer();

    virtual void OnAccept(int client);
    virtual void OnConnect();

    static const float m_cSPF;
    static const char* m_cClassName;
    static const char* m_cWindowName;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};

#endif // __GAME_H__

