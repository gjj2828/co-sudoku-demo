#ifndef __SOCKOBJ_H__
#define __SOCKOBJ_H__

#include "ISockObj.h"

class CSockObj : public ISockObj
{
public:
    CSockObj(int id, ESockType type, SOCallBack callback);
    ~CSockObj();

    virtual int     GetId() {return m_iId;}
    virtual void    SetId(int id) {m_iId = id;};
    virtual int     Bind(const sockaddr* addr, int namelen);
    virtual int     Listen(int backlog);
    virtual int     Accept();
    virtual int     Connect();
    virtual int     Send(Packet* packet);
    virtual int     Recv();

private:
    enum EEvent
    {
        EEVENT_MIN,
        EEVENT_ACCPET = EEVENT_MIN,
        EEVENT_CONNECT = EEVENT_ACCPET,
        EEVENT_SEND,
        EEVENT_RECV,
        EEVENT_MAX,
    };
    enum ETcpType
    {
        ETCPTYPE_MIN,
        ETCPTYPE_UNDECIDED = ETCPTYPE_MIN,
        ETCPTYPE_LISTEN,
        ETCPTYPE_ACCEPT,
        ETCPTYPE_CONNECT,
        ETCPTYPE_MAX,
    };
    int                         m_iId;
    ESockType                   m_eSockType;
    ETcpType                    m_eTcpType;
    SOCKET                      m_Sock;
    HANDLE                      m_hEvents[EEVENT_MAX];
    WSAOVERLAPPED               m_Overlapped[EEVENT_MAX];
    LPFN_ACCEPTEX               m_lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;
    SOCallBack                  m_CallBack;
    CSockObj*                   m_AcceptSO;
};

#endif // __SOCKOBJ_H__