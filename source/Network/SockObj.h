#ifndef __SOCKOBJ_H__
#define __SOCKOBJ_H__

#include <INetworkEvent.h>
#include "ISockObj.h"

class CSockObj : public ISockObj
{
public:
    CSockObj(int id, INetworkEventManager* event_manager);
    ~CSockObj();

    virtual int Create(ESockType type);
    virtual int GetId() {return m_iId;}
    virtual int Bind(SOCKADDR* addr, int namelen);
    virtual int Listen(int backlog);
    virtual int PostAccept(ISockObj* accept, char* buf, int len);
    virtual int PostConnect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len);
    virtual int PostSend(Packet* packet, SOCKADDR* addr, int namelen);
    virtual int PostRecv();
    virtual int Update();

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

    int                         m_iId;
    ESockType                   m_eSockType;
    //ETcpType                    m_eTcpType;
    SOCKET                      m_Sock;
    HANDLE                      m_hEvents[EEVENT_MAX];
    WSAOVERLAPPED               m_Overlapped[EEVENT_MAX];
    LPFN_ACCEPTEX               m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;
    INetworkEventManager*       m_pEventManager;
    char*                       m_pAcceptBuf;
    char*                       m_pAcceptBufOrg;
    int                         m_iAcceptBufLen;
    SendDataQue                 m_queSendData;
    bool                        m_bSending;
    ERecvStep                   m_eRecvStep;
    int                         m_iOffset;
    psize_t                     m_iSize;
    Packet*                     m_pRecvPacket;

	SOCKET GetSocket() {return m_Sock;}

    int PostSend();
    int PostEvent(INetworkEventManager::EEvent event, int ret = NO_ERROR, ISockObj* accept = NULL, Packet* recv = NULL);

	int OnAccept();
	int OnConnect();
	int OnSend();
	int OnRecv();
};

#endif // __SOCKOBJ_H__