#include "StdAfx.h"
#include "NetworkSystem.h"

#define WAITFAILEDRTN(rc)   \
{                           \
    if((rc) == WAIT_FAILED) \
    {                       \
        Stop();             \
        return;             \
    }                       \
}

const char* CNetworkSystem::m_cBroadCastSendAddr            = "192.168.255.255";
const float CNetworkSystem::m_cBroadCastSendInterval        = 1.0f;
const float CNetworkSystem::m_cBroadCastCheckServerInterval = 1.0f;

CNetworkSystem::CNetworkSystem()
: m_eState(ESTATE_STOPPED)
, m_iSendCount(0)
, m_bBroadCastSending(false)
, m_bBroadCastRecving(false)
, m_fBroadCastSendTime(0.0f)
, m_bClientSending(false)
, m_bCheckServer(false)
, m_fCheckServerTime(0.0f)
, m_bHost(false)
, m_ibcRecAddrSize(0)
, m_soBroadCast(INVALID_SOCKET)
, m_soListener(INVALID_SOCKET)
, m_soClient(INVALID_SOCKET)
, m_iCoClientMax(COCLIENT_MAX)
, m_iCoClientNum(0)
, m_bRecvServer(false)
{
    ZeroMemory(m_hEvents, sizeof(HANDLE) * EEVENT_MAX);
    for(int i = 0; i < COCLIENT_MAX; i++)
    {
        m_CoClientData[i].s = INVALID_SOCKET;
    }
}

int CNetworkSystem::Init()
{
    WSADATA wsd;

    if(WSAStartup(MAKEWORD(2, 2), &wsd)) ERROR_RTN0("Can\'t load Winsock!");

    m_bcBindAddr.sin_family         = AF_INET;
    m_bcBindAddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    m_bcBindAddr.sin_port           = htons(BROADCAST_PORT);

    m_bcSendAddr.sin_family         = AF_INET;
    m_bcSendAddr.sin_addr.s_addr    = inet_addr(m_cBroadCastSendAddr);
    m_bcSendAddr.sin_port           = htons(BROADCAST_PORT);

    m_BindAddr.sin_family           = AF_INET;
    m_BindAddr.sin_addr.s_addr      = htonl(INADDR_ANY);
    m_BindAddr.sin_port             = htons(MAIN_PORT);

    m_ibcRecAddrSize                = sizeof(m_bcRecvAddr);

    for(int i = 0; i < EEVENT_MAX; i++)
    {
        m_Overlapped[i].hEvent = WSACreateEvent();
        m_hEvents[i] = m_Overlapped[i].hEvent;
    }

    return 1;
}

void CNetworkSystem::Release()
{
    WSACleanup();

    this->~CNetworkSystem();
}

void CNetworkSystem::Update(float time)
{
    if(m_eState == ESTATE_STOPPED) return;

    int rc;
    DWORD dwBytes, dwFlags;

    typedef std::vector<int> ToDeleteVector;
    ToDeleteVector vectorToDelete;

    rc = WaitForMultipleObjects(EEVENT_MAX, m_hEvents, false, 0);
    WAITFAILEDRTN(rc);

    if(rc != WAIT_TIMEOUT)
    {
        rc = WaitForSingleObject(m_hEvents[EEVENT_BROADCAST_SEND], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_BROADCAST_SEND]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_BROADCAST_SEND], &dwBytes, false, &dwFlags);
            if(!OnBroadCastSend(rc)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_BROADCAST_RECV], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_BROADCAST_RECV]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_BROADCAST_RECV], &dwBytes, false, &dwFlags);
            if(!OnBroadCastRecv(rc, dwBytes)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_ACCEPT], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_ACCEPT]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_ACCEPT], &dwBytes, false, &dwFlags);
            if(!OnAccept(rc)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_CONNECT], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_CONNECT]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_BROADCAST_SEND], &dwBytes, false, &dwFlags);
            if(!OnConnect(rc)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_CLIENTSEND], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_CLIENTSEND]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_CLIENTSEND], &dwBytes, false, &dwFlags);
            if(!OnClientSend(rc)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_CLIENTRECV], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_CLIENTRECV]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_CLIENTRECV], &dwBytes, false, &dwFlags);
            if(!OnClientRecv(rc, dwBytes)) return;
        }

        for(int i = 0; i < COCLIENT_MAX; i++)
        {
            rc = WaitForSingleObject(m_hEvents[EEVENT_HOSTSEND_MIN + i], 0);
            WAITFAILEDRTN(rc);
            if(rc == WAIT_OBJECT_0)
            {
                WSAResetEvent(m_hEvents[EEVENT_HOSTSEND_MIN + i]);
                rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_HOSTSEND_MIN + i], &dwBytes, false, &dwFlags);
                if(!OnHostSend(rc, i)) return;
            }

            rc = WaitForSingleObject(m_hEvents[EEVENT_HOSTRECV_MIN + i], 0);
            WAITFAILEDRTN(rc);
            if(rc == WAIT_OBJECT_0)
            {
                WSAResetEvent(m_hEvents[EEVENT_HOSTRECV_MIN + i]);
                rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_HOSTRECV_MIN + i], &dwBytes, false, &dwFlags);
                if(!OnHostRecv(rc, dwBytes, i)) return;
            }
        }
    }

    CheckBroadCastServer(time);
    if(!CheckBroadCastSend()) return;

    return NO_ERROR;
}

int CNetworkSystem::Start(EMode mode, float time)
{
    PRINT("Network start!\n");
    if(StartInternal(mode, time) == NO_ERROR) return 1;
    Stop();
    return 0;
}

void CNetworkSystem::Stop()
{
    m_eState = ESTATE_STOPPED;

    SAFE_CLOSESOCKET(m_soBroadCast);
    SAFE_CLOSESOCKET(m_soListener);
    SAFE_CLOSESOCKET(m_soClient);
    for(int i = 0; i < COCLIENT_MAX; i++)
    {
        SAFE_CLOSESOCKET(m_CoClientData[i].s);
    }
}

int CNetworkSystem::StartInternal(EMode mode, float time)
{
    if(m_eState != ESTATE_STOPPED) return -1;

    int rc;
    DWORD dwBytes;

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    m_iSendCount    = 0;
    m_eState        = ESTATE_WAITING;

    for(int i = 0; i < EEVENT_MAX; i++)
    {
        WSAResetEvent(m_hEvents[i]);
    }

    m_soListener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_soListener == INVALID_SOCKET) return -1;

    rc = bind(m_soListener, (SOCKADDR*)&m_BindAddr, sizeof(m_BindAddr));
    if(rc == SOCKET_ERROR) return WSAGetLastError();

    rc = WSAIoctl( m_soListener
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidAcceptEx
        , sizeof(guidAcceptEx)
        , &m_lpfnAcceptEx
        , sizeof(m_lpfnAcceptEx)
        , &dwBytes
        , NULL
        , NULL );
    if(rc == SOCKET_ERROR) return WSAGetLastError();

    rc = WSAIoctl( m_soListener
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidGetAcceptExSockaddrs
        , sizeof(guidGetAcceptExSockaddrs)
        , &m_lpfnGetAcceptExSockaddrs
        , sizeof(m_lpfnGetAcceptExSockaddrs)
        , &dwBytes
        , NULL
        , NULL );
    if(rc == SOCKET_ERROR) return WSAGetLastError();

    PRINT("listening...\n");

    rc = listen(m_soListener, 5);
    if(rc == SOCKET_ERROR) return WSAGetLastError();

    m_iCoClientNum = 0;

    rc = PostAccept();
    if(rc != NO_ERROR) return rc;

    m_soBroadCast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(m_soBroadCast == INVALID_SOCKET) return -1;

    rc = bind(m_soBroadCast, (SOCKADDR*)&m_bcBindAddr, sizeof(m_bcBindAddr));
    if(rc == SOCKET_ERROR) return WSAGetLastError();

    rc = PostBroadCastSend(time);
    if(rc != NO_ERROR) return rc;

    rc = PostBroadCastRecv();
    if(rc != NO_ERROR) return rc;

    StartCheckServer(time);

    return NO_ERROR;
}

int CNetworkSystem::PostBroadCastSend(float time)
{
    PRINT("PostBroadCastSend\n");

    if(m_bBroadCastSending) return NO_ERROR;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes;

    strcpy(m_bcSendBuf.sGameName, GAME_NAME);
    m_bcSendBuf.iSendCount  = m_iSendCount;
    m_bcSendBuf.iState      = m_eState;
    m_bcSendBuf.iClientNum  = 0;
    m_iSendCount++;

    buf.buf = (char*)&m_bcSendBuf;
    buf.len = sizeof(m_bcSendBuf);
    rc = WSASendTo(m_soBroadCast, &buf, 1, &dwBytes, 0, (SOCKADDR*)&m_bcSendAddr, sizeof(m_bcSendAddr), &m_Overlapped[EEVENT_BROADCAST_SEND], NULL);
    if(rc == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    m_bBroadCastSending = true;
    m_fBroadCastSendTime = time;

    return NO_ERROR;
}

int CNetworkSystem::PostBroadCastRecv()
{
    PRINT("PostBroadCastRecv\n");

    if(m_bBroadCastRecving) return NO_ERROR;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes, dwFlags;

    buf.buf = (char*)&m_bcRecvBuf;
    buf.len = sizeof(m_bcRecvBuf);
    dwFlags = 0;
    rc = WSARecvFrom(m_soBroadCast, &buf, 1, &dwBytes, &dwFlags, (SOCKADDR*)&m_bcRecvAddr, &m_ibcRecAddrSize, &m_Overlapped[EEVENT_BROADCAST_RECV], NULL);
    if(rc == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}

int CNetworkSystem::PostAccept()
{
    PRINT("PostAccept\n");

    int rc;
    DWORD dwBytes;

    m_CoClientData[m_iCoClientNum].s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_CoClientData[m_iCoClientNum].s == INVALID_SOCKET) return -1;

    rc = m_lpfnAcceptEx(m_soListener, m_CoClientData[m_iCoClientNum].s, m_AcceptBuf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]);
    if(rc == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}

int CNetworkSystem::PostConnect()
{
    PRINT("PostConnect\n");

    int             rc;
    DWORD           dwBytes;
    SOCKADDR_IN     ServerAddr;
    LPFN_CONNECTEX  lpfnConnectEx;

    GUID guidConnectEx = WSAID_CONNECTEX;

    m_soClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_soClient == INVALID_SOCKET) return -1;

    rc = bind(m_soClient, (SOCKADDR*)&m_BindAddr, sizeof(m_BindAddr));
    if(rc == SOCKET_ERROR) return WSAGetLastError();

    rc = WSAIoctl( m_soClient
                 , SIO_GET_EXTENSION_FUNCTION_POINTER
                 , &guidConnectEx
                 , sizeof(guidConnectEx)
                 , &lpfnConnectEx
                 , sizeof(lpfnConnectEx)
                 , &dwBytes
                 , NULL
                 , NULL );
    if(rc == SOCKET_ERROR) return WSAGetLastError();

    ServerAddr.sin_family   = AF_INET;
    ServerAddr.sin_addr     = m_Server;
    ServerAddr.sin_port     = htons(MAIN_PORT);

    rc = lpfnConnectEx(m_soClient, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr), NULL, 0, &dwBytes, &m_Overlapped[EEVENT_CONNECT]);
    if(rc == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
}

int CNetworkSystem::PostClientSend()
{
    PRINT("PostClientSend\n");

    if(m_bBroadCastSending) return NO_ERROR;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes;

    strcpy(m_bcSendBuf.sGameName, GAME_NAME);
    m_bcSendBuf.iSendCount  = m_iSendCount;
    m_bcSendBuf.iState      = m_eState;
    m_bcSendBuf.iClientNum  = 0;
    m_iSendCount++;

    buf.buf = (char*)&m_bcSendBuf;
    buf.len = sizeof(m_bcSendBuf);
    rc = WSASendTo(m_soBroadCast, &buf, 1, &dwBytes, 0, (SOCKADDR*)&m_bcSendAddr, sizeof(m_bcSendAddr), &m_Overlapped[EEVENT_BROADCAST_SEND], NULL);
    if(rc == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    m_bBroadCastSending = true;
    m_fBroadCastSendTime = time;

    return NO_ERROR;
}

int CNetworkSystem::PostClientRecv()
{
    PRINT("PostClientRecv\n");

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes, dwFlags;

    buf.buf = (char*)&m_RecvBuf;
    buf.len = sizeof(m_RecvBuf);
    dwFlags = 0;
    rc = WSARecv(m_soClient, &buf, 1, &dwBytes, &dwFlags, &m_Overlapped[EEVENT_CLIENTRECV], NULL);
    if(rc == SOCKET_ERROR)
    {
        rc = rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
};

int CNetworkSystem::PostHostSend(int client)
{
    return NO_ERROR;
}

int CNetworkSystem::PostHostRecv(int client)
{
    PRINT("PostHostRecv\n");

    if(client >= m_iCoClientMax) return -1;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes, dwFlags;

    CoClinetData* pData = &m_CoClientData[client];

    buf.buf = (char*)&pData->RecvBuf;
    buf.len = sizeof(pData->RecvBuf);
    dwFlags = 0;
    rc = WSARecv(m_CoClientData[client].s, &buf, 1, &dwBytes, &dwFlags, &m_Overlapped[EEVENT_HOSTRECV_MIN + client], NULL);
    if(rc == SOCKET_ERROR)
    {
        rc = rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
};

int CNetworkSystem::OnBroadCastSend(int rc, float time)
{
    PRINT("OnBroadCastSend!");
    m_bBroadCastSending = false;
    return 1
}

int CNetworkSystem::OnBroadCastRecv(int rc, float time, DWORD bytes)
{
    PRINT("OnBroadCastRecv %s!", inet_ntoa(m_bcRecvAddr.sin_addr));

    m_bBroadCastRecving = false;

    if(!m_bCheckServer) return;

    if(strcmp(m_bcRecvBuf.sGameName, GAME_NAME) != 0) return;

    bool bUpdateServer = false;

    if(m_bRecvServer)
    {
        if(m_bcRecvAddr.sin_addr.s_addr == m_Server.s_addr) return;
        switch(m_bcRecvBuf.iState)
        {
        case ESTATE_STOPPED:
            break;
        case ESTATE_WAITING:
            if(m_ServerData.iState == ESTATE_WAITING)
            {
                if(m_bcRecvBuf.iSendCount > m_ServerData.iSendCount)
                {
                    bUpdateServer = true;
                }
                else if(m_bcRecvBuf.iSendCount == m_ServerData.iSendCount)
                {
                    if(m_bcRecvAddr.sin_addr.s_addr < m_Server.s_addr) bUpdateServer = true;
                }
            }
            break;
        case ESTATE_PAIRED:
            if(m_ServerData.iState == ESTATE_WAITING)
            {
                bUpdateServer = true;
            }
            else if(m_ServerData.iState == ESTATE_PAIRED)
            {
                if(m_bcRecvBuf.iClientNum > m_ServerData.iClientNum)
                {
                    bUpdateServer = true;
                }
                else if(m_bcRecvBuf.iClientNum == m_ServerData.iClientNum)
                {
                    if(m_bcRecvBuf.iSendCount > m_ServerData.iSendCount)
                    {
                        bUpdateServer = true;
                    }
                    else if(m_bcRecvBuf.iSendCount == m_ServerData.iSendCount)
                    {
                        if(m_bcRecvAddr.sin_addr.s_addr < m_Server.s_addr) bUpdateServer = true;
                    }
                }
            }
            break;
        default:
            assert(0);
            break;
        }
    }
    else
    {
        if(m_bcRecvBuf.iSendCount > m_iSendCount)
        {
            bUpdateServer = true;
        }
        else if(m_bcRecvBuf.iSendCount == m_iSendCount)
        {
            int         rc;
            DWORD       dwBytes;
            SOCKADDR_IN LocalIf;
            rc = WSAIoctl( m_soBroadCast
                         , SIO_ROUTING_INTERFACE_QUERY
                         , &m_bcRecvAddr
                         , sizeof(m_bcRecvAddr)
                         , (SOCKADDR *)&LocalIf
                         , sizeof(LocalIf)
                         , &dwBytes
                         , NULL
                         , NULL );
            if(rc == SOCKET_ERROR) return;
            if(m_bcRecvAddr.sin_addr.s_addr < LocalIf.sin_addr.s_addr) bUpdateServer = true;
        }
    }

    if(bUpdateServer)
    {
        m_Server        = m_bcRecvAddr.sin_addr;
        m_ServerData    = m_bcRecvBuf;
        m_bRecvServer   = true;
    }

    PostBroadCastRecv();

    return 1;
}

int CNetworkSystem::OnAccept(int rc, float time)
{
    PRINT("OnAccept!");
    if(rc == TRUE)
    {
        SOCKADDR*   LocalSockaddr;
        SOCKADDR*   RemoteSockaddr;
        int         LocalSockaddrLen;
        int         RemoteSockaddrLen;
        m_lpfnGetAcceptExSockaddrs( m_AcceptBuf, sizeof(m_AcceptBuf) - (sizeof(SOCKADDR_IN) + 16) * 2, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16
                                  , (SOCKADDR **)&LocalSockaddr, &LocalSockaddrLen, (SOCKADDR **)&RemoteSockaddr, &RemoteSockaddrLen );
        if(m_eState == ESTATE_WAITING || (m_eState == ESTATE_PAIRED && m_bHost == true))
        {
            if(strcmp(m_AcceptBuf, GAME_NAME) == 0)
            {
                rc = PostHostRecv(m_iCoClientNum);
                if(rc == NO_ERROR)
                {
                    if(m_eState == ESTATE_WAITING)
                    {
                        m_eState = ESTATE_PAIRED;
                        m_bHost = true;
                    }
                    m_iCoClientNum++;
                    if(m_iCoClientNum < m_iCoClientMax)
                    {
                        rc = PostAccept();
                        if(rc != NO_ERROR) return rc;
                    }
                }
                else
                {
                    SAFE_CLOSESOCKET(m_CoClientData[m_iCoClientNum].s);
                    rc = PostAccept();
                    if(rc != NO_ERROR) return rc;
                }
            }
            else
            {
                SAFE_CLOSESOCKET(m_CoClientData[m_iCoClientNum].s);
                rc = PostAccept();
                if(rc != NO_ERROR) return rc;
            }
        }
        else
        {
            SAFE_CLOSESOCKET(m_CoClientData[m_iCoClientNum].s);
        }
    }
    else
    {
        SAFE_CLOSESOCKET(m_CoClientData[m_iCoClientNum].s);
        rc = PostAccept();
        if(rc != NO_ERROR) return rc;
    }

    return 1;
}

int CNetworkSystem::OnConnect(int rc, float time)
{
    PRINT("OnConnect!");

    if(rc == TRUE)
    {
        int opval = 1;
        rc = setsockopt(m_soClient, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, (char*)&opval, sizeof(opval));
        if(rc == SOCKET_ERROR)
        {
            SAFE_CLOSESOCKET(m_soClient);
            m_eState
        }
        m_eState = ESTATE_PAIRED;
        m_bHost = false;
        rc = PostClientRecv();
    }
    return 1;
}

int CNetworkSystem::OnClientSend(int rc, float time)
{
    return 1;
}

int CNetworkSystem::OnClientRecv(int rc, float time, DWORD bytes)
{
    return 1;
}

int CNetworkSystem::OnHostSend(int rc, float time, int client)
{
    return 1;
}

int CNetworkSystem::OnHostRecv(int rc, float time, DWORD bytes, int client)
{
    return 1;
}

void CNetworkSystem::StartCheckServer(float time)
{
    m_fCheckServerTime    = time;
    m_bCheckServer        = true;
    m_bRecvServer           = false;
}

void CNetworkSystem::StopCheckServer()
{
    m_bCheckServer = false;
}

void CNetworkSystem::CheckBroadCastServer(float time)
{
    if(!m_bCheckServer)                                               return;
    if(time - m_fCheckServerTime < m_cBroadCastCheckServerInterval)   return;

    int rc;

    m_fCheckServerTime = time;

    if(m_eState == ESTATE_WAITING)
    {
        if(m_bRecvServer)
        {
            SAFE_CLOSESOCKET(m_soListener);
            rc = PostConnect();
            if(rc == NO_ERROR)
            {
                m_eState = ESTATE_CONNECTING;
                StopCheckServer();
            }
            else
            {
                SAFE_CLOSESOCKET(m_soClient);
                m_bRecvServer = false;
            }
        }
    }
}

int CNetworkSystem::CheckBroadCastSend(float time)
{
    if(time - m_fBroadCastSendTime < m_cBroadCastSendInterval) return 1;

    if((m_eState == ESTATE_WAITING) || (m_eState == ESTATE_PAIRED && m_bHost = true && m_iCoClientNum < m_iCoClientMax))
    {
        int rc;
        m_fBroadCastSendTime = time;
        rc = PostBroadCastSend(time);
        if(rc != NO_ERROR)
        {
            Stop();
            return 0;
        }
    }

    return 1;
}