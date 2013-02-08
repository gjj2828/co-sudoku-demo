#ifndef __NETWORKSYSTEM_H__
#define __NETWORKSYSTEM_H__

#include <INetworkSystem.h>
#include <MSWSock.h>
#include "ISockObj.h"
#include "INetworkEventManager.h"

class CNetworkSystem: public INetworkSystem, public INetworkEventManager
{
public:
    CNetworkSystem();
    virtual int     Init();
    virtual void    Release();
    virtual int     Start(EMode mode, float time);
    virtual void    Stop();
    virtual int     Update(float time);
    virtual void    Send(Packet* packet);
    virtual void    Send(Packet* packet, int client);
    virtual void    RegisterListener(INetworkListener* listener);
    virtual void    UnRegisterListener(INetworkListener* listener);

private:
    enum EState
    {
        ESTATE_MIN,
        ESTATE_STOP = ESTATE_MIN,
        ESTATE_WAITING,
        ESTATE_CONNECTING,
        ESTATE_HOST,
        ESTATE_CLIENT,
        ESTATE_MAX,
    };

    enum
    {
        LISTEN_PORT     = 7778,
        CLIENT_PORT     = 7779,
        BROADCAST_PORT  = 7780,
        COCLIENT_MAX    = 16,
        GAMENAME_LEN    = 16,
    };

    enum ESockObjType
    {
        ESOCKOBJTYPE_MIN,
        ESOCKOBJTYPE_LISTEN = ESOCKOBJTYPE_MIN,
        ESOCKOBJTYPE_BROADCAST,
        ESOCKOBJTYPE_CLIENT,
        ESOCKOBJTYPE_COCLIENT_MIN = ESOCKOBJTYPE_CLIENT,
        ESOCKOBJTYPE_COCLIENT_MAX = ESOCKOBJTYPE_COCLIENT_MIN + COCLIENT_MAX,
        ESOCKOBJTYPE_MAX = ESOCKOBJTYPE_COCLIENT_MAX,
    };

    enum EPacketType
    {
        EPACKETTYPE_MIN,
        EPACKETTYPE_BROADCAST = EPACKETTYPE_MIN,
        EPACKETTYPE_MAX,
    };

    struct BroadCastPacket : public Packet
    {
        BroadCastPacket()
        {
            size = sizeof(*this);
            type = EPACKETTYPE_BROADCAST;
        }
        char    sGameName[GAMENAME_LEN];
        int     iSendCount;
        UINT    iState          :  2;
        UINT    iCoClientNum    :  5;
        UINT    iDummy          : 25;
    };

    typedef std::vector<INetworkListener*> ListenerVector;

    EState                      m_eState;
    float                       m_fTime;
    int                         m_iSendCount;
    float                       m_fBroadCastSendTime;

    float                       m_fRecvServerTime;
    bool                        m_bRecvServer;
    IN_ADDR                     m_Server;
    BroadCastPacket             m_ServerPacket;

    UINT                        m_iCoClientMax;
    UINT                        m_iCoClientNum;

    SOCKADDR_IN                 m_BroadCastBindAddr;
    SOCKADDR_IN                 m_BroadCastSendAddr;
    SOCKADDR_IN                 m_BroadCastRecvAddr;
    SOCKADDR_IN                 m_ListenBindAddr;
    SOCKADDR_IN                 m_ClientBindAddr;

    ISockObj*                   m_pSockObjs[ESOCKOBJTYPE_MAX];

    ListenerVector              m_vectorListener;

    virtual int HandleEvent(const Event& event);

    void        BroadCastCheckInfo();
    int         BroadCastSendInfo();

    bool        FindEmptyCoClient(int& empty);

    void        ChangeState(EState state);

    ISockObj*   CreateSockObj(int id);
    void        FreeSockObj(ISockObj* obj);

    void        OnAccept(int client);
    void        OnConnect();
    void        OnRecv(Packet* pPacket);
    void        OnDisconnect();
    void        OnStop();

    static const float m_cBroadCastSendInterval;
    static const float m_cBroadCastCheckServerInterval;
};

#endif // __NETWORKSYSTEM_H__