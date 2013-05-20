#ifndef __SOCKOBJ_H__
#define __SOCKOBJ_H__

#include <MSWSock.h>
#include "INetworkEventManager.h"
#include "ISockObj.h"

class TSockObj : public ISockObj
{
public:
    TSockObj(int id, INetworkEventManager* event_manager);

    virtual int     GetId() {return m_iId;}
    virtual int     Listen(SOCKADDR* addr, int namelen, int buf_len, int backlog, bool is_broadcast) = 0;
    virtual int     Accept(SOCKET sock) {return -1;}
    virtual int     Connect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len) {return -1;}
    virtual int     Send(Packet* packet, SOCKADDR* addr, int namelen);
    virtual int     Update();
    virtual void    Close();
    virtual void    Release();

protected:
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

    struct SendData
    {
        SendData() : packet(NULL), namelen(0) {}
        ~SendData() {SAFE_DELETE(packet);}
        Packet*     packet;
        SOCKADDR    addr;
        int         namelen;
    };

    typedef std::deque<SendData*> SendDataQue;

    SOCKET                      m_soMain;
    char*                       m_pBuf;
    int                         m_iBufLen;
    HANDLE                      m_hEvents[EEVENT_MAX];
    WSAOVERLAPPED               m_Overlapped[EEVENT_MAX];
    bool                        m_bSending;
    bool                        m_bRecving;
    SendDataQue                 m_queSendData;

    int PostSend();
    int PostEvent( INetworkEventManager::EEvent event, int ret = NO_ERROR, SOCKADDR* local = NULL, SOCKADDR* remote = NULL
        , Packet* packet = NULL, char* buf = NULL, int buf_len = 0, SOCKET sock = INVALID_SOCKET );

private:
    typedef int (TSockObj::*EventFunc)();

    int                     m_iId;
    INetworkEventManager*   m_pEventManager;
    EventFunc               m_pEventFunc[EEVENT_MAX];

    virtual int PostAccept()    {return -1;}
    virtual int OnAccept()      {return -1;}
    virtual int OnConnect()     {return -1;}
    virtual int OnSend()        {return -1;}
	virtual int OnRecv()        {return -1;}

    virtual int PostSendData(SendData* data_ptr)    = 0;
    virtual int PostRecv()                          = 0;
    virtual int OnRecv(int bytes)                   = 0;

};

class CTcpSockObj : public TSockObj
{
public:
    CTcpSockObj(int id, INetworkEventManager* event_manager);

    virtual int     Listen(SOCKADDR* addr, int namelen, int buf_len, int backlog, bool is_broadcast);
    virtual int     Accept(SOCKET sock);
    virtual int     Connect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len);
    virtual void    Close();

private:
    SOCKET                      m_soAccept;
    LPFN_ACCEPTEX               m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;
    ERecvStep                   m_eRecvStep;
    int                         m_iOffset;
    psize_t                     m_iSize;
    Packet*                     m_pRecvPacket;

    virtual int PostAccept();
    virtual int OnAccept();
    virtual int OnConnect();
    virtual int OnSend();
    virtual int OnRecv();
    virtual int PostSendData(SendData* data_ptr);
    virtual int PostRecv();
    virtual int OnRecv(int bytes);

    int SetKeepAlive();
};

class CUdpSockObj : public TSockObj
{
public:
    CUdpSockObj(int id, INetworkEventManager* event_manager);

    virtual int Listen(SOCKADDR* addr, int namelen, int buf_len, int backlog, bool is_broadcast);

private:
    SOCKADDR    m_RecvAddr;
    int         m_iRecvAddrSize;

    virtual int OnSend();
    virtual int OnRecv();
    virtual int PostSendData(SendData* data_ptr);
    virtual int PostRecv();
    virtual int OnRecv(int bytes);
};

#endif // __SOCKOBJ_H__