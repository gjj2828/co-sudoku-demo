#ifndef __SOCKOBJ_H__
#define __SOCKOBJ_H__

#include <INetworkEvent.h>
#include "ISockObj.h"

class CSockObj : public ISockObj
{
public:
    CSockObj(int id, INetworkEventManager* event_manager);
    ~CSockObj();

    virtual int     Create(ESockType type);
    virtual int     GetId() {return m_iId;}
    virtual int     Bind(const sockaddr* addr, int namelen);
    virtual int     Listen(int backlog);
    virtual int     PostAccept(ISockObj* accept, char* buf, int len);
    virtual int     PostConnect(const sockaddr* addr, int namelen);
    virtual int     PostSend(Packet* packet);
    virtual int     PostRecv();

private:
    enum EEvent
    {
        EEVENT_MIN,
        EEVENT_ACCEPT = EEVENT_MIN,
        EEVENT_CONNECT = EEVENT_ACCEPT,
        EEVENT_SEND,
        EEVENT_RECV,
        EEVENT_MAX,
    };
    //enum ETcpType
    //{
    //    ETCPTYPE_MIN,
    //    ETCPTYPE_UNDECIDED = ETCPTYPE_MIN,
    //    ETCPTYPE_LISTEN,
    //    ETCPTYPE_ACCEPT,
    //    ETCPTYPE_CONNECT,
    //    ETCPTYPE_MAX,
    //};

    int                         m_iId;
    ESockType                   m_eSockType;
    //ETcpType                    m_eTcpType;
    SOCKET                      m_Sock;
    HANDLE                      m_hEvents[EEVENT_MAX];
    WSAOVERLAPPED               m_Overlapped[EEVENT_MAX];
    LPFN_ACCEPTEX               m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;
    INetworkEventManager*       m_pEventManager;
    CSockObj*                   m_pAcceptSO;
    char*                       m_pBuf;
    int                         m_iBufLen;

    int     CreateI(ESockType type);
    int     BindI(const sockaddr* addr, int namelen);
    int     ListenI(int backlog);
    int     PostAcceptI(ISockObj* accept, char* buf, int len);
    int     PostConnectI(const sockaddr* addr, int namelen);
    int     PostSendI(Packet* packet);
    int     PostRecvI();
    void    PostEvent(INetworkEventManager::EEvent event, int ret, ISockObj* accept, Packet* recv);
};

#endif // __SOCKOBJ_H__