#ifndef __SOCKOBJ_H__
#define __SOCKOBJ_H__

#include <MSWSock.h>
#include "INetworkEventManager.h"
#include "ISockObj.h"

class CSockObj : public ISockObj
{
public:
    CSockObj(int id, INetworkEventManager* event_manager);
    ~CSockObj();

    virtual int     GetId() {return m_iId;}
    virtual int     Listen(ESockType type, SOCKADDR* addr, int namelen, int buf_len, int backlog);
    virtual int     Accept(SOCKET sock);
    virtual int     Connect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len);
    virtual int     Send(Packet* packet, SOCKADDR* addr, int namelen);
    virtual int     Update();
    virtual void    Close();

private:

private:
    enum EEvent
    {
        EEVENT_MIN,
        EEVENT_ACCEPT = EEVENT_MIN,
        EEVENT_CONNECT,
        EEVENT_SEND,
        EEVENT_RECV,
        EEVENT_MAX,
    };
    enum ERecvStep
    {
        ERECVSTEP_MIN,
        ERECVSTEP_SIZE = ERECVSTEP_MIN,
        ERECVSTEP_PACKET,
        ERECVSTEP_MAX,
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

    struct SendData
    {
        SendData() : packet(NULL), namelen(0) {}
        ~SendData() {SAFE_DELETE(packet);}
        Packet*     packet;
        SOCKADDR    addr;
        int         namelen;
    };

    typedef std::deque<SendData*> SendDataQue;
    typedef int (CSockObj::*EventFunc)();

    int                         m_iId;
    ESockType                   m_eSockType;
    //ETcpType                    m_eTcpType;
    SOCKET                      m_soMain;
    HANDLE                      m_hEvents[EEVENT_MAX];
    WSAOVERLAPPED               m_Overlapped[EEVENT_MAX];
    LPFN_ACCEPTEX               m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;
    INetworkEventManager*       m_pEventManager;
    SOCKET                      m_soAccept;
    char*                       m_pBuf;
    int                         m_iBufLen;
    SendDataQue                 m_queSendData;
    bool                        m_bConnecting;
    bool                        m_bSending;
    bool                        m_bRecving;
    ERecvStep                   m_eRecvStep;
    int                         m_iOffset;
    psize_t                     m_iSize;
    Packet*                     m_pRecvPacket;
    SOCKADDR                    m_RecvAddr;
    int                         m_iRecvAddrSize;
    EventFunc                   m_pEventFunc[EEVENT_MAX];

    int PostAccept();
    int PostSend();
    int PostRecv();
    int PostEvent( INetworkEventManager::EEvent event, int ret = NO_ERROR, SOCKADDR* local = NULL, SOCKADDR* remote = NULL
                 , Packet* packet = NULL, char* buf = NULL, int buf_len = 0, SOCKET sock = INVALID_SOCKET );
	int OnAccept();
	int OnConnect();
	int OnSend();
	int OnRecv();
    int OnRecv(int bytes);
};

#endif // __SOCKOBJ_H__