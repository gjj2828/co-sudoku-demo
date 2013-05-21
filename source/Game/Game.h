//**************************************************
//File: Game.h
//Author: GaoJiongjiong
//Function: ÓÎÏ·Ö÷Ìå
//**************************************************

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
        EPACKETTYPE_TEST1 = EPACKETTYPE_MIN,
        EPACKETTYPE_TEST2,
        EPACKETTYPE_MAX,
    };

    struct Test1Packet : public Packet
    {
        Test1Packet()
        {
            size = sizeof(*this);
            type = EPACKETTYPE_TEST1;
        }

        float val;
    };

    struct Test2Packet : public Packet
    {
        Test2Packet()
        {
            size = sizeof(*this);
            type = EPACKETTYPE_TEST2;
        }

        float val;
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
    void    Update();
    int     InitTimer();
    void    UpdateTimer();

    virtual void OnAccept(int client);
    virtual void OnConnect();
    virtual void OnRecv(Packet* pPacket);
    virtual void OnDisconnect();
    virtual void OnStop();

    static const float m_cSPF;
    static const char* m_cClassName;
    static const char* m_cWindowName;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};

#endif // __GAME_H__

