#include "StdAfx.h"
#include "SockObj.h"

#define POSTEVENT_RTN(event, rc)                    \
{                                                   \
    int _rc = (rc);                                 \
    PostEvent(INetworkEventManager::event, _rc);    \
    return _rc;                                     \
}

CSockObj::CSockObj(int id, ESockType type, INetworkEventManager* event_manager)
 : m_iId(id)
 , m_eSockType(ESOCKTYPE_UNDECIDED)
 , m_Sock(INVALID_SOCKET)
 , m_lpfnAcceptEx(NULL)
 , m_lpfnGetAcceptExSockaddrs(NULL)
 , m_pEventManager(event_manager)
 , m_pAcceptSO(NULL)
 , m_pAcceptBuf(NULL)
 , m_pAcceptBufOrg(NULL)
 , m_iAcceptBufLen(0)
 , m_bSending(false)
 , m_eRecvStep(ERECVSTEP_MIN)
 , m_iOffset(0)
 , m_iSize(0)
 , m_pRecvPacket(NULL)
{
    for(int i = 0; i < EEVENT_MAX; i++)
    {
        m_Overlapped[i].hEvent = WSACreateEvent();
        m_hEvents[i] = m_Overlapped[i].hEvent;
    }
}

CSockObj::~CSockObj()
{
    Close();
    for(int i = 0; i < EEVENT_MAX; i++)
    {
        WSACloseEvent(m_hEvents[i])
    }
}

int CSockObj::Create(ESockType type)
{
    int iRetCode = CreateI(type);
    if(iRetCode != NO_ERROR) PostEvent(INetworkEventManager::EEVENT_CREATEFAIL, iRetCode);
    return iRetCode;
}

void CSockObj::Close()
{
    SAFE_CLOSESOCKET(m_Sock);

    m_eSockType                 = ESOCKTYPE_UNDECIDED;
    m_lpfnAcceptEx              = NULL;
    m_lpfnGetAcceptExSockaddrs  = NULL;
    m_pEventManager             = NULL;
    m_pAcceptSO                 = NULL;
    m_pAcceptBuf                = NULL;
    m_pAcceptBufOrg             = NULL;
    m_iAcceptBufLen             = 0;
    m_bSending                  = false;
    m_eRecvStep                 = ERECVSTEP_MIN;
    m_iOffset                   = 0;
    m_iSize                     = 0;

    for(int i = 0; i < m_queSendPacket.size(); i++)
    {
        SAFE_DELETE(m_queSendPacket[i]);
    }
    m_queSendPacket.clear();

    SAFE_DELETE(m_pRecvPacket);
}

int CSockObj::Bind(SOCKADDR* addr, int namelen)
{
    if(bind(m_Sock, addr, namelen) == SOCKET_ERROR) POSTEVENT_RTN(EEVENT_BINDFAIL, WSAGetLastError());
    return NO_ERROR;
}

int CSockObj::Listen(int backlog)
{
    DWORD dwBytes;

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    if(m_eSockType != ESOCKTYPE_TCP) POSTEVENT_RTN(EEVENT_LISTENFAIL, -1);

    if(listen(m_Sock, backlog) == SOCKET_ERROR) POSTEVENT_RTN(EEVENT_LISTENFAIL, WSAGetLastError());

    if(WSAIoctl( m_Sock
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidAcceptEx
        , sizeof(guidAcceptEx)
        , &m_lpfnAcceptEx
        , sizeof(m_lpfnAcceptEx)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) POSTEVENT_RTN(EEVENT_LISTENFAIL, WSAGetLastError());

    if(WSAIoctl( m_Sock
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidGetAcceptExSockaddrs
        , sizeof(guidGetAcceptExSockaddrs)
        , &m_lpfnGetAcceptExSockaddrs
        , sizeof(m_lpfnGetAcceptExSockaddrs)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) POSTEVENT_RTN(EEVENT_LISTENFAIL, WSAGetLastError());

    return NO_ERROR;
}

int CSockObj::PostAccept(ISockObj* accept, char* buf, int len)
{
    int rc;
    DWORD dwBytes;

    if(m_eSockType != ESOCKTYPE_TCP)                    POSTEVENT_RTN(EEVENT_POSTACCEPTFAIL, -1);
    if(!m_lpfnAcceptEx || !m_lpfnGetAcceptExSockaddrs)  POSTEVENT_RTN(EEVENT_POSTACCEPTFAIL, -1);
    if(!m_pAcceptBuf)                                   POSTEVENT_RTN(EEVENT_POSTACCEPTFAIL, -1);

    m_pAcceptSO = accept;

    m_pAcceptBufOrg   = buf;
    m_iAcceptBufLen   = len;
    m_pAcceptBuf      = malloc(m_iAcceptBufLen + (sizeof(SOCKADDR_IN) + 16) * 2);

    if(m_lpfnAcceptEx(m_Sock, m_pAcceptSO, m_pAcceptBuf, m_iAcceptBufLen, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) POSTEVENT_RTN(EEVENT_POSTACCEPTFAIL, rc);
    }

    return NO_ERROR;
}

int CSockObj::PostConnect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen)
{
    int             rc;
    DWORD           dwBytes;
    SOCKADDR_IN     ServerAddr;
    LPFN_CONNECTEX  lpfnConnectEx;

    GUID guidConnectEx = WSAID_CONNECTEX;

    if(m_eSockType != ESOCKTYPE_TCP) POSTEVENT_RTN(EEVENT_POSTCONNECTFAIL, -1);

    if(bind(m_Sock, local_addr, local_namelen) == SOCKET_ERROR) POSTEVENT_RTN(EEVENT_POSTCONNECTFAIL, WSAGetLastError());

    if( WSAIoctl( m_Sock
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidConnectEx
        , sizeof(guidConnectEx)
        , &lpfnConnectEx
        , sizeof(lpfnConnectEx)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) POSTEVENT_RTN(EEVENT_POSTCONNECTFAIL, WSAGetLastError());

    if(lpfnConnectEx(m_Sock, &remote_addr, remote_namelen, buf, len, &dwBytes, &m_Overlapped[EEVENT_CONNECT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) POSTEVENT_RTN(EEVENT_POSTCONNECTFAIL, rc);
    }

    return NO_ERROR;
}

int CSockObj::PostSend(Packet* packet, SOCKADDR* addr, int namelen)
{
    Packet* pPacket = (Packet*)malloc(packet->size);
    memcpy(pPacket, (char*)packet, packet->size);
    m_queSendPacket.push_back(pPacket);

    return PostSend(addr, namelen);
}

int CSockObj::PostSend(SOCKADDR* addr, int namelen)
{
    if(m_eSockType == ESOCKTYPE_UNDECIDED) return -1;
    if(m_bSending) return NO_ERROR;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes;

    while(m_queSendPacket.size() > 0)
    {
        Packet* pPacket = m_queSendPacket.front();

        buf.buf = (char*)pPacket;
        buf.len = pPacket->size;

        switch(m_eSockType)
        {
        case ESOCKTYPE_TCP:
            rc = WSASend(m_Sock, &buf, 1, &dwBytes, 0, &m_Overlapped[EEVENT_SEND], NULL);
            break;
        case ESOCKTYPE_UDP:
            rc = WSASendTo(m_Sock, &buf, 1, &dwBytes, 0, addr, namelen, &m_Overlapped[EEVENT_SEND], NULL);
            break;
        default:
            assert(0);
            return -1;
        }

        if(rc == SOCKET_ERROR)
        {
            rc = WSAGetLastError();
            if(rc != WSA_IO_PENDING) return rc;
            break;
        }

        WSAResetEvent(m_Overlapped[EEVENT_SEND].hEvent);
        m_queSendPacket.pop_front();
        SAFE_DELETE(pPacket);
    }

    if(m_queSendPacket.size() > 0) m_bSending = true;

    return NO_ERROR;
}

int CSockObj::PostRecv()
{
    int iRetCode = PostRecvI();
    if(iRetCode != NO_ERROR) PostEvent(INetworkEventManager::EEVENT_POSTRECVTFAIL, iRetCode);
    return iRetCode;
}

int CSockObj::CreateI(ESockType type)
{
    if(m_eSockType != ESOCKTYPE_UNDECIDED) return -1;

    switch(type)
    {
        case ESOCKTYPE_TCP:
            m_Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(m_Sock == INVALID_SOCKET) return -1;
            break;
        case ESOCKTYPE_UDP:
            m_Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(m_Sock == INVALID_SOCKET) return -1;
            break;
        default:
            assert(0);
            return -1;
    }
    m_eSockType = type;

    for(int i = 0; i < EEVENT_MAX; i++)
    {
        WSAResetEvent(m_hEvents[i]);
    }

    return NO_ERROR;
}

int CSockObj::BindI(SOCKADDR* addr, int namelen)
{
}

int CSockObj::ListenI(int backlog)
{
}

int CSockObj::PostAcceptI(ISockObj* accept, char* buf, int len)
{
    int rc;
    DWORD dwBytes;

    if(m_eSockType != ESOCKTYPE_TCP)                    return -1;
    if(!m_lpfnAcceptEx || !m_lpfnGetAcceptExSockaddrs)  return -1;
    if(!m_pAcceptBuf)                                         return -1;

    m_pAcceptSO = accept;

    m_pAcceptBufOrg   = buf;
    m_iAcceptBufLen   = len;
    m_pAcceptBuf      = malloc(m_iAcceptBufLen + (sizeof(SOCKADDR_IN) + 16) * 2);

    if(m_lpfnAcceptEx(m_Sock, m_pAcceptSO, m_pAcceptBuf, m_iAcceptBufLen, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}

int CSockObj::PostConnectI(SOCKADDR* local_addr, int local_namelen, SOCKADDR* remote_addr, int remote_namelen, char* buf, int len)
{
    int             rc;
    DWORD           dwBytes;
    SOCKADDR_IN     ServerAddr;
    LPFN_CONNECTEX  lpfnConnectEx;

    GUID guidConnectEx = WSAID_CONNECTEX;

    if(m_eSockType != ESOCKTYPE_TCP)                    return -1;

    if(bind(m_Sock, local_addr, local_namelen) == SOCKET_ERROR) return WSAGetLastError();

    if(WSAIoctl( m_Sock
               , SIO_GET_EXTENSION_FUNCTION_POINTER
               , &guidConnectEx
               , sizeof(guidConnectEx)
               , &lpfnConnectEx
               , sizeof(lpfnConnectEx)
               , &dwBytes
               , NULL
               , NULL ) == SOCKET_ERROR) return WSAGetLastError();

    if(lpfnConnectEx(m_Sock, &remote_addr, remote_namelen, buf, len, &dwBytes, &m_Overlapped[EEVENT_CONNECT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}

int CSockObj::PostSend(SOCKADDR* addr, int namelen)
{
    int iRetCode = PostSendI(addr, namelen);
    if(iRetCode != NO_ERROR) PostEvent(INetworkEventManager::EEVENT_POSTSENDFAIL, iRetCode);
    return iRetCode;
}

int CSockObj::PostRecvI()
{
    WSABUF  buf;
    int     rc;
    DWORD   dwBytes, dwFlags;

    while(1)
    {
        switch(m_eRecvStep)
        {
        case ERECVSTEP_SIZE:
            buf.buf = (char*)&m_iSize + m_iOffset;
            buf.len = sizeof(psize_t) - m_iOffset;
            break;
        case ERECVSTEP_PACKET:
            buf.buf = (char*)m_pRecvPacket + m_iOffset;
            buf.len = m_pRecvPacket->size - m_iOffset;
            break;
        }

        dwFlags = 0;
        rc = WSARecv(m_Sock, &buf, 1, &dwBytes, &dwFlags, &m_Overlapped[EEVENT_RECV], NULL);
        if(rc == SOCKET_ERROR)
        {
            rc = WSAGetLastError();
            if(rc != WSA_IO_PENDING) return rc;
            break;
        }

        if(dwBytes == 0)
        {
            PostEvent(INetworkEventManager::EEVENT_BINDFAIL, NO_ERROR);
        }

        switch(m_eRecvStep)
        {
        case ERECVSTEP_SIZE:
            if(m_iOffset + dwBytes < sizeof(psize_t))
            buf.buf = (char*)&m_iSize + m_iOffset;
            buf.len = sizeof(psize_t) - m_iOffset;
            break;
        case ERECVSTEP_PACKET:
            buf.buf = (char*)m_pRecvPacket + m_iOffset;
            buf.len = m_pRecvPacket->size - m_iOffset;
            break;
        }
    }

    return NO_ERROR;
}