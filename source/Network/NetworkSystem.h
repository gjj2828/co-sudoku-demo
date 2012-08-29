#ifndef __NETWORKSYSTEM_H__
#define __NETWORKSYSTEM_H__

#include <INetworkSystem.h>
#include <MSWSock.h>

class CNetworkSystem: public INetworkSystem
{
public:
    CNetworkSystem();
    virtual int     Init();
    virtual void    Release();
    virtual void    Update(float time);
    virtual int     Start(EMode mode, float time);
    virtual void    Stop();
    virtual void    RegisterListener(INetworkListener* listener) {}
    virtual void    UnRegisterListener(INetworkListener* listener) {}

private:
    enum EState
    {
        ESTATE_MIN,
        ESTATE_STOPPED = ESTATE_MIN,
        ESTATE_WAITING,
        ESTATE_CONNECTING,
        ESTATE_PAIRED,
        ESTATE_MAX,
    };

    enum
    {
        MAIN_PORT       = 7778,
        BROADCAST_PORT  = 7779,
        COCLIENT_MAX    = 16,
        GAMENAME_LEN    = 16,
    };

    enum EEvent
    {
        EEVENT_MIN,
        EEVENT_BROADCAST_SEND = EEVENT_MIN,
        EEVENT_BROADCAST_RECV,
        EEVENT_ACCEPT,
        EEVENT_CONNECT = EEVENT_ACCEPT,
        EEVENT_CLIENTSEND,
        EEVENT_CLIENTRECV,
        EEVENT_HOSTSEND_MIN = EEVENT_CLIENTSEND,
        EEVENT_HOSTSEND_MAX = EEVENT_HOSTSEND_MIN + COCLIENT_MAX,
        EEVENT_HOSTRECV_MIN = EEVENT_HOSTSEND_MAX,
        EEVENT_HOSTRECV_MAX = EEVENT_HOSTRECV_MIN + COCLIENT_MAX,
        EEVENT_MAX = EEVENT_HOSTRECV_MAX,
    };


    struct BroadCastData
    {
        char    sGameName[GAMENAME_LEN];
        int     iSendCount;
        UINT    iState      :  2;
        UINT    iClientNum  :  5;
        UINT    iDummy      : 25;
    };

    struct GameData
    {
        int choice;
    };

    typedef std::deque<GameData*> GameDataQue;

    struct CoClinetData
    {
        SOCKET      s;
        GameData    RecvBuf;
        GameDataQue SendBufQue;
    };

    EState                      m_eState;
    int                         m_iSendCount;
    bool                        m_bBroadCastSending;
    bool                        m_bBroadCastRecving;
    float                       m_fBroadCastSendTime;
    bool                        m_bClientSending;
    bool                        m_bHost;

    SOCKADDR_IN                 m_bcBindAddr;
    SOCKADDR_IN                 m_bcSendAddr;
    SOCKADDR_IN                 m_bcRecvAddr;
    SOCKADDR_IN                 m_BindAddr;

    int                         m_ibcRecAddrSize;

    SOCKET                      m_soBroadCast;
    SOCKET                      m_soListener;
    SOCKET                      m_soClient;

    CoClinetData                m_CoClientData[COCLIENT_MAX];

    LPFN_ACCEPTEX               m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;

    int                         m_iCoClientMax;
    int                         m_iCoClientNum;

    HANDLE                      m_hEvents[EEVENT_MAX];
    WSAOVERLAPPED               m_Overlapped[EEVENT_MAX];

    BroadCastData               m_bcSendBuf;
    BroadCastData               m_bcRecvBuf;
    char                        m_AcceptBuf[GAMENAME_LEN + (sizeof(SOCKADDR_IN) + 16) * 2];
    GameData                    m_RecvBuf;
    GameDataQue                 m_SendBufQue;

    bool                        m_bCheckServer;
    float                       m_fCheckServerTime;
    bool                        m_bRecvServer;
    in_addr                     m_Server;
    BroadCastData               m_ServerData;

    int     StartInternal(EMode mode, float time);

    int     PostBroadCastSend(float time);
    int     PostBroadCastRecv();
    int     PostAccept();
    int     PostConnect();
    int     PostClientSend();
    int     PostClientRecv();
    int     PostHostSend(int client);
    int     PostHostRecv(int client);

    int     OnBroadCastSend(int rc, float time);
    int     OnBroadCastRecv(int rc, float time, DWORD bytes);
    int     OnAccept(int rc, float time);
    int     OnConnect(int rc, float time);
    int     OnClientSend(int rc, float time);
    int     OnClientRecv(int rc, float time, DWORD bytes);
    int     OnHostSend(int rc, float time, int client);
    int     OnHostRecv(int rc, float time, DWORD bytes, int client);

    void    StartCheckServer(float time);
    void    StopCheckServer();
    void    CheckBroadCastServer(float time);
    int     CheckBroadCastSend(float time);

    static const char* m_cBroadCastSendAddr;
    static const float m_cBroadCastSendInterval;
    static const float m_cBroadCastCheckServerInterval;
};

#endif // __NETWORKSYSTEM_H__