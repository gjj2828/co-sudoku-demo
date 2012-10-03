#include "StdAfx.h"
#include "SockObj.h"

CSockObj::CSockObj(int id, ESockType type, SOCallBack callback)
 : m_iId(id)
 , m_eSockType(type)
 , m_eTcpType(ETCPTYPE_UNDECIDED)
 , m_Sock(INVALID_SOCKET)
 , m_CallBack(callback)
 , m_AcceptSO(NULL)
{
    for(int i = 0; i < EEVENT_MAX; i++)
    {
        m_Overlapped[i].hEvent = WSACreateEvent();
        m_hEvents[i] = m_Overlapped[i].hEvent;
    }
    switch(type)
    {
    case ESOCKTYPE_TCP:
        m_Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        break;
    case ESOCKTYPE_UDP:
        m_Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        break;
    default:
        assert(0);
        break;
    }
}

CSockObj::~CSockObj()
{
    closesocket(m_Sock);
    for(int i = 0; i < EEVENT_MAX; i++)
    {
        WSACloseEvent(m_hEvents[i])
    }
}

int CSockObj::Bind(const sockaddr* addr, int namelen)
{
    if(bind(m_Sock, addr, namelen) == SOCKET_ERROR) return WSAGetLastError();
    return NO_ERROR;
}

int CSockObj::Listen(int backlog)
{
    DWORD dwBytes;

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    if(m_eSockType != ESOCKTYPE_TCP) return -1;
    if(m_eTcpType != ETCPTYPE_UNDECIDED) return -1;

    if(listen(m_Sock, backlog) == SOCKET_ERROR) return WSAGetLastError();

    if(WSAIoctl( m_Sock
               , SIO_GET_EXTENSION_FUNCTION_POINTER
               , &guidAcceptEx
               , sizeof(guidAcceptEx)
               , &m_lpfnAcceptEx
               , sizeof(m_lpfnAcceptEx)
               , &dwBytes
               , NULL
               , NULL ) == SOCKET_ERROR) return WSAGetLastError();

    if(WSAIoctl( m_Sock
               , SIO_GET_EXTENSION_FUNCTION_POINTER
               , &guidGetAcceptExSockaddrs
               , sizeof(guidGetAcceptExSockaddrs)
               , &m_lpfnGetAcceptExSockaddrs
               , sizeof(m_lpfnGetAcceptExSockaddrs)
               , &dwBytes
               , NULL
               , NULL ) == SOCKET_ERROR) return WSAGetLastError();

    m_eTcpType = ETCPTYPE_LISTEN;

    return NO_ERROR;
}

int CSockObj::Accept()
{
    int rc;
    DWORD dwBytes;

    if(m_eSockType != ESOCKTYPE_TCP)    return -1;
    if(m_eTcpType != ETCPTYPE_LISTEN)   return -1;
    if(m_AcceptSO)                      return -1;

    m_AcceptSO = new CSockObj(-1, );
    if(m_soAccept == INVALID_SOCKET) return -1;

    if(m_lpfnAcceptEx(m_Sock, m_soAccept, m_AcceptBuf, GAMENAME_LEN, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}