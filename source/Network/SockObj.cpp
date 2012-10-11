#include "StdAfx.h"
#include "SockObj.h"

CSockObj::CSockObj(int id, ESockType type, INetworkEventManager* event_manager)
 : m_iId(id)
 , m_eSockType(ESOCKTYPE_UNDECIDED)
 , m_Sock(INVALID_SOCKET)
 , m_lpfnAcceptEx(NULL)
 , m_lpfnGetAcceptExSockaddrs(NULL)
 , m_pEventManager(event_manager)
 , m_pAcceptSO(NULL)
 , m_pBuf(NULL)
 , m_iBufLen(0)
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
    m_lpfnAcceptEx = NULL;
    m_lpfnGetAcceptExSockaddrs = NULL;
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

int CSockObj::PostConnect(const sockaddr* addr, int namelen)
{
    int iRetCode = PostConnectI(addr, namelen);
    if(iRetCode != NO_ERROR) PostEvent(INetworkEventManager::EEVENT_POSTCONNECTFAIL, iRetCode);
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

int CSockObj::PostAcceptI(ISockObj* accept, char* buf, int len)
{
    int rc;
    DWORD dwBytes;

    if(m_eSockType != ESOCKTYPE_TCP)                    return -1;
    if(!m_lpfnAcceptEx || !m_lpfnGetAcceptExSockaddrs)  return -1;
    if(!m_pBuf)                                         return -1;

    m_pAcceptSO = accept;

    m_iBufLen = len;
    m_pBuf = malloc(m_iBufLen + (sizeof(SOCKADDR_IN) + 16) * 2);

    if(m_lpfnAcceptEx(m_Sock, m_pAcceptSO, buf, len, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}

int CSockObj::PostConnectI(const sockaddr* addr, int namelen)
{
    int             rc;
    DWORD           dwBytes;
    SOCKADDR_IN     ServerAddr;
    LPFN_CONNECTEX  lpfnConnectEx;

    GUID guidConnectEx = WSAID_CONNECTEX;

    m_soClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_soClient == INVALID_SOCKET) return -1;

    if(bind(m_soClient, (SOCKADDR*)&m_LocalBindAddr, sizeof(m_LocalBindAddr)) == SOCKET_ERROR) return WSAGetLastError();

    if(WSAIoctl( m_soClient
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidConnectEx
        , sizeof(guidConnectEx)
        , &lpfnConnectEx
        , sizeof(lpfnConnectEx)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) return WSAGetLastError();

    ServerAddr.sin_family   = AF_INET;
    ServerAddr.sin_addr     = m_Server;
    ServerAddr.sin_port     = htons(MAIN_PORT);

    if(lpfnConnectEx(m_soClient, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr), m_ConnectBuf, GAMENAME_LEN, &dwBytes, &m_Overlapped[EEVENT_CONNECT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}
