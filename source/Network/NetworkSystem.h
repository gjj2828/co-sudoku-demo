#ifndef __NETWORKSYSTEM_H__
#define __NETWORKSYSTEM_H__

#include <INetworkSystem.h>
#include <INetworkEvent.h>
#include <MSWSock.h>

class CNetworkSystem: public INetworkSystem, public INetworkEventManager
{
public:
    CNetworkSystem();
    virtual int     Init();
    virtual void    Release();
    virtual int     Start(EMode mode, float time);
    virtual void    Stop();
    virtual void    Update(float time);
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

    //enum EEvent
    //{
    //    EEVENT_MIN,
    //    EEVENT_BROADCAST_SEND = EEVENT_MIN,
    //    EEVENT_BROADCAST_RECV,
    //    EEVENT_ACCEPT,
    //    EEVENT_CONNECT,
    //    EEVENT_CLIENTSEND,
    //    EEVENT_CLIENTRECV,
    //    EEVENT_HOSTSEND_MIN = EEVENT_CLIENTSEND,
    //    EEVENT_HOSTSEND_MAX = EEVENT_HOSTSEND_MIN + COCLIENT_MAX,
    //    EEVENT_HOSTRECV_MIN = EEVENT_HOSTSEND_MAX,
    //    EEVENT_HOSTRECV_MAX = EEVENT_HOSTRECV_MIN + COCLIENT_MAX,
    //    EEVENT_MAX = EEVENT_HOSTRECV_MAX,
    //};

    struct BroadCastData
    {
        char    sGameName[GAMENAME_LEN];
        int     iSendCount;
        UINT    iState      :  2;
        UINT    iCoClientNum  :  5;
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
        bool        bSending;
    };

    EState                      m_eState;
    int                         m_iSendCount;
    bool                        m_bBroadCastSending;
    float                       m_fBroadCastSendTime;
    bool                        m_bClientSending;
    bool                        m_bHost;

    SOCKADDR_IN                 m_BroadCastBindAddr;
    SOCKADDR_IN                 m_BroadCastSendAddr;
    SOCKADDR_IN                 m_BroadCastRecvAddr;
    SOCKADDR_IN                 m_LocalBindAddr;

    int                         m_iBroadCastRecAddrSize;

    SOCKET                      m_soBroadCast;
    SOCKET                      m_soListener;
    SOCKET                      m_soAccept;
    SOCKET                      m_soClient;

    CoClinetData                m_CoClientData[COCLIENT_MAX];

    LPFN_ACCEPTEX               m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;

    int                         m_iCoClientMax;
    int                         m_iCoClientNum;

    HANDLE                      m_hEvents[EEVENT_MAX];
    WSAOVERLAPPED               m_Overlapped[EEVENT_MAX];

    BroadCastData               m_BroadCastSendBuf;
    BroadCastData               m_BroadCastRecvBuf;
    char                        m_AcceptBuf[GAMENAME_LEN + (sizeof(SOCKADDR_IN) + 16) * 2];
    char                        m_ConnectBuf[GAMENAME_LEN];
    GameData                    m_ClientRecvBuf;
    GameDataQue                 m_ClientSendBufQue;

    bool                        m_bCheckServer;
    float                       m_fCheckServerTime;
    bool                        m_bRecvServer;
    in_addr                     m_Server;
    BroadCastData               m_ServerData;

    int     PostBroadCastSend(float time);
    int     PostBroadCastRecv();
    int     PostAccept();
    int     PostConnect();
    int     PostClientSend();
    int     PostClientRecv();
    int     PostHostSend(int client);
    int     PostHostRecv(int client);

    int     OnBroadCastSend(float time);
    int     OnBroadCastRecv(float time);
    int     OnAccept(float time);
    int     OnConnect(float time);
    int     OnClientSend(float time);
    int     OnClientRecv(float time);
    int     OnHostSend(float time, int client);
    int     OnHostRecv(float time, int client);

    void    StartClient();
    void    StopClient(float time);
    void    StartCoClient(int client);
    void    StopCoClient(int client);

    void    StartCheckServer(float time);
    void    StopCheckServer();
    void    CheckBroadCastServer(float time);
    int     CheckBroadCastSend(float time);

    bool    FindEmptyCoClient(int& empty);

    void    ReleaseInternal();
    void    ReleaseCoClientData(int client);

    static const char* m_cBroadCastSendAddr;
    static const float m_cBroadCastSendInterval;
    static const float m_cBroadCastCheckServerInterval;
};

#endif // __NETWORKSYSTEM_H__