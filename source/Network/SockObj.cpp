#include "StdAfx.h"
#include "SockObj.h"

CSockObj::CSockObj(int id, ESockType type, INetworkEventManager* event_manager)
 : m_iId(id)
 , m_eSockType(ESOCKTYPE_UNDECIDED)
 , m_Sock(INVALID_SOCKET)
 , m_pEventManager(event_manager)
 , m_pAcceptSO(NULL)
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
    m_eSockType = ESOCKTYPE_UNDECIDED;
}

int CSockObj::Bind(const sockaddr* addr, int namelen)
{
    int iRetCode = BindI(addr, namelen);
    if(iRetCode != NO_ERROR) PostEvent(INetworkEventManager::EEVENT_BINDFAIL, iRetCode);
    return iRetCode;
}

int CSockObj::Listen(int backlog)
{
    int iRetCode = ListenI(backlog);
    if(iRetCode != NO_ERROR) PostEvent(INetworkEventManager::EEVENT_LISTENFAIL, iRetCode);
    return iRetCode;
}

int CSockObj::PostAccept(ISockObj* accept, char* buf, int len)
{
    int iRetCode = PostAcceptI(accept, buf, len);
    if(iRetCode != NO_ERROR) PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, iRetCode);
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

int CSockObj::BindI(const sockaddr* addr, int namelen)
{
    if(bind(m_Sock, addr, namelen) == SOCKET_ERROR) return WSAGetLastError();
    return NO_ERROR;
}

int CSockObj::ListenI(int backlog)
{
    DWORD dwBytes;

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    if(m_eSockType != ESOCKTYPE_TCP) return -1;

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

    return NO_ERROR;
}

int CSockObj::PostAccept(ISockObj* accept, char* buf, int len)
{
    int rc;
    DWORD dwBytes;

    if(m_eSockType != ESOCKTYPE_TCP)    return -1;

    m_pAcceptSO = accept;

    if(m_lpfnAcceptEx(m_Sock, m_soAccept, m_AcceptBuf, GAMENAME_LEN, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}