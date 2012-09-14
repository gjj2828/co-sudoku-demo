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
, m_iBroadCastRecAddrSize(0)
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
        m_CoClientData[i].bSending = false;
    }
}

int CNetworkSystem::Init()
{
    WSADATA wsd;

    if(WSAStartup(MAKEWORD(2, 2), &wsd)) ERROR_RTN0("Can\'t load Winsock!");

    m_BroadCastBindAddr.sin_family         = AF_INET;
    m_BroadCastBindAddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    m_BroadCastBindAddr.sin_port           = htons(BROADCAST_PORT);

    m_BroadCastSendAddr.sin_family         = AF_INET;
    m_BroadCastSendAddr.sin_addr.s_addr    = inet_addr(m_cBroadCastSendAddr);
    m_BroadCastSendAddr.sin_port           = htons(BROADCAST_PORT);

    m_LocalBindAddr.sin_family           = AF_INET;
    m_LocalBindAddr.sin_addr.s_addr      = htonl(INADDR_ANY);
    m_LocalBindAddr.sin_port             = htons(MAIN_PORT);

    m_iBroadCastRecAddrSize                = sizeof(m_BroadCastRecvAddr);

    for(int i = 0; i < EEVENT_MAX; i++)
    {
        m_Overlapped[i].hEvent = WSACreateEvent();
        m_hEvents[i] = m_Overlapped[i].hEvent;
    }

    strcpy(m_BroadCastSendBuf.sGameName, GAME_NAME);
    strcpy(m_ConnectBuf, GAME_NAME);

    return 1;
}

void CNetworkSystem::Release()
{

    ReleaseInternal();
    WSACleanup();
    this->~CNetworkSystem();
}

void CNetworkSystem::Update(float time)
{
    if(m_eState == ESTATE_STOPPED) return;

    int rc;

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
            if(!OnBroadCastSend(time)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_BROADCAST_RECV], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_BROADCAST_RECV]);
            if(!OnBroadCastRecv(time)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_ACCEPT], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_ACCEPT]);
            if(!OnAccept(time)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_CONNECT], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_CONNECT]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_BROADCAST_SEND], &dwBytes, false, &dwFlags);
            if(!OnConnect(time)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_CLIENTSEND], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_CLIENTSEND]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_CLIENTSEND], &dwBytes, false, &dwFlags);
            if(!OnClientSend(time)) return;
        }

        rc = WaitForSingleObject(m_hEvents[EEVENT_CLIENTRECV], 0);
        WAITFAILEDRTN(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_CLIENTRECV]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_CLIENTRECV], &dwBytes, false, &dwFlags);
            if(!OnClientRecv(time)) return;
        }

        for(int i = 0; i < COCLIENT_MAX; i++)
        {
            rc = WaitForSingleObject(m_hEvents[EEVENT_HOSTSEND_MIN + i], 0);
            WAITFAILEDRTN(rc);
            if(rc == WAIT_OBJECT_0)
            {
                WSAResetEvent(m_hEvents[EEVENT_HOSTSEND_MIN + i]);
                rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_HOSTSEND_MIN + i], &dwBytes, false, &dwFlags);
                if(!OnHostSend(time, i)) return;
            }

            rc = WaitForSingleObject(m_hEvents[EEVENT_HOSTRECV_MIN + i], 0);
            WAITFAILEDRTN(rc);
            if(rc == WAIT_OBJECT_0)
            {
                WSAResetEvent(m_hEvents[EEVENT_HOSTRECV_MIN + i]);
                rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_HOSTRECV_MIN + i], &dwBytes, false, &dwFlags);
                if(!OnHostRecv(time, i)) return;
            }
        }
    }

    CheckBroadCastServer(time);
    if(!CheckBroadCastSend()) return;
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
    PRINT("Network stop!\n");
    m_eState = ESTATE_STOPPED;
    ReleaseInternal();
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

    if(bind(m_soListener, (SOCKADDR*)&m_LocalBindAddr, sizeof(m_LocalBindAddr)) == SOCKET_ERROR) return WSAGetLastError();

    if(WSAIoctl( m_soListener
               , SIO_GET_EXTENSION_FUNCTION_POINTER
               , &guidAcceptEx
               , sizeof(guidAcceptEx)
               , &m_lpfnAcceptEx
               , sizeof(m_lpfnAcceptEx)
               , &dwBytes
               , NULL
               , NULL ) == SOCKET_ERROR) return WSAGetLastError();

    if(WSAIoctl( m_soListener
               , SIO_GET_EXTENSION_FUNCTION_POINTER
               , &guidGetAcceptExSockaddrs
               , sizeof(guidGetAcceptExSockaddrs)
               , &m_lpfnGetAcceptExSockaddrs
               , sizeof(m_lpfnGetAcceptExSockaddrs)
               , &dwBytes
               , NULL
               , NULL ) == SOCKET_ERROR) return WSAGetLastError();

    PRINT("listening...\n");

    if(listen(m_soListener, 5) == SOCKET_ERROR) return WSAGetLastError();

    m_iCoClientNum = 0;

    rc = PostAccept();
    if(rc != NO_ERROR) return rc;

    m_soBroadCast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(m_soBroadCast == INVALID_SOCKET) return -1;

    if(bind(m_soBroadCast, (SOCKADDR*)&m_BroadCastBindAddr, sizeof(m_BroadCastBindAddr)) == SOCKET_ERROR) return WSAGetLastError();

    rc = StartCheckServer(time);
    if(rc != NO_ERROR) return rc;

    return NO_ERROR;
}

int CNetworkSystem::PostBroadCastSend(float time)
{
    PRINT("PostBroadCastSend\n");

    if(m_bBroadCastSending) return NO_ERROR;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes;

    m_BroadCastSendBuf.iSendCount   = m_iSendCount;
    m_BroadCastSendBuf.iState       = m_eState;
    m_BroadCastSendBuf.iCoClientNum = m_iCoClientNum;
    m_iSendCount++;

    buf.buf = (char*)&m_BroadCastSendBuf;
    buf.len = sizeof(m_BroadCastSendBuf);

    if(WSASendTo(m_soBroadCast, &buf, 1, &dwBytes, 0, (SOCKADDR*)&m_BroadCastSendAddr, sizeof(m_BroadCastSendAddr), &m_Overlapped[EEVENT_BROADCAST_SEND], NULL) == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING)
        {
            Stop();
            return rc;
        }
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

    buf.buf = (char*)&m_BroadCastRecvBuf;
    buf.len = sizeof(m_BroadCastRecvBuf);

    dwFlags = 0;
    if(WSARecvFrom(m_soBroadCast, &buf, 1, &dwBytes, &dwFlags, (SOCKADDR*)&m_BroadCastRecvAddr, &m_iBroadCastRecAddrSize, &m_Overlapped[EEVENT_BROADCAST_RECV], NULL) == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING)
        {
            Stop();
            return rc;
        }
    }

    return NO_ERROR;
}

int CNetworkSystem::PostAccept()
{
    PRINT("PostAccept\n");

    if(!FindEmptyCoClient(m_iCoClinetAccept)) return NO_ERROR;

    int rc;
    DWORD dwBytes;

    m_CoClientData[m_iCoClinetAccept].s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_CoClientData[m_iCoClinetAccept].s == INVALID_SOCKET) return -1;

    if(m_lpfnAcceptEx(m_soListener, m_CoClientData[m_iCoClinetAccept].s, m_AcceptBuf, GAMENAME_LEN, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING)
        {
            Stop();
            return rc;
        }
    }

    return NO_ERROR;
}

int CNetworkSystem::PostConnect(float time)
{
    PRINT("PostConnect\n");

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
        if(rc != WSA_IO_PENDING)
        {
            StopClient(time);
            return rc;
        }
    }

    return NO_ERROR;
}

int CNetworkSystem::PostClientSend(float time)
{
    PRINT("PostClientSend\n");

    if(m_bClientSending) return NO_ERROR;
    if(m_ClientSendBufQue.size() == 0) return NO_ERROR;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes;

    GameData* pData = m_ClientSendBufQue.front();

    buf.buf = (char*)pData;
    buf.len = sizeof(GameData);

    if(WSASend(m_soClient, &buf, 1, &dwBytes, 0, &m_Overlapped[EEVENT_CLIENTSEND], NULL) == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING)
        {
            StopClient(time);
            return rc;
        }
    }

    m_bClientSending = true;

    return NO_ERROR;
}

int CNetworkSystem::PostClientRecv(float time)
{
    PRINT("PostClientRecv\n");

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes, dwFlags;

    buf.buf = (char*)&m_ClientRecvBuf;
    buf.len = sizeof(m_ClientRecvBuf);

    dwFlags = 0;
    if(WSARecv(m_soClient, &buf, 1, &dwBytes, &dwFlags, &m_Overlapped[EEVENT_CLIENTRECV], NULL) == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING)
        {
            StopClient(time);
            return rc;
        }
    }

    return NO_ERROR;
};

int CNetworkSystem::PostHostSend(int client)
{
    PRINT("PostHostSend client: %d\n", client);

    if(m_bClientSending) return NO_ERROR;
    if(m_ClientSendBufQue.size() == 0) return NO_ERROR;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes;

    GameData* pData = m_ClientSendBufQue.front();

    buf.buf = (char*)pData;
    buf.len = sizeof(GameData);

    if(WSASend(m_soClient, &buf, 1, &dwBytes, 0, &m_Overlapped[EEVENT_CLIENTSEND], NULL) == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    m_bClientSending = true;

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
    if(WSARecv(m_CoClientData[client].s, &buf, 1, &dwBytes, &dwFlags, &m_Overlapped[EEVENT_HOSTRECV_MIN + client], NULL) == SOCKET_ERROR)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return rc;
    }

    return NO_ERROR;
};

int CNetworkSystem::OnBroadCastSend(int rc, float time)
{
    PRINT("OnBroadCastSend!");

    int rc;
    DWORD dwBytes, dwFlags;

    WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_BROADCAST_SEND], &dwBytes, false, &dwFlags);

    m_bBroadCastSending = false;
    return 1;
}

int CNetworkSystem::OnBroadCastRecv(int rc, float time, DWORD bytes)
{
    PRINT("OnBroadCastRecv %s!", inet_ntoa(m_BroadCastRecvAddr.sin_addr));

    int rc;
    DWORD dwBytes, dwFlags;

    rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_BROADCAST_RECV], &dwBytes, false, &dwFlags);

    m_bBroadCastRecving = false;

    if(PostBroadCastRecv() != NO_ERROR) return 0;
    if(!m_bCheckServer) return 1;

    bool bUpdateServer = false;
    if(rc == TRUE && dwBytes == sizeof(BroadCastData) && strcmp(m_BroadCastRecvBuf.sGameName, GAME_NAME) == 0)
    {
        if(m_bRecvServer)
        {
            if(m_BroadCastRecvAddr.sin_addr.s_addr == m_Server.s_addr) return 1;
            switch(m_BroadCastRecvBuf.iState)
            {
            case ESTATE_STOPPED:
                break;
            case ESTATE_WAITING:
                if(m_ServerData.iState == ESTATE_WAITING)
                {
                    if(m_BroadCastRecvBuf.iSendCount > m_ServerData.iSendCount)
                    {
                        bUpdateServer = true;
                    }
                    else if(m_BroadCastRecvBuf.iSendCount == m_ServerData.iSendCount)
                    {
                        if(m_BroadCastRecvAddr.sin_addr.s_addr < m_Server.s_addr) bUpdateServer = true;
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
                    if(m_BroadCastRecvBuf.iCoClientNum > m_ServerData.iCoClientNum)
                    {
                        bUpdateServer = true;
                    }
                    else if(m_BroadCastRecvBuf.iCoClientNum == m_ServerData.iCoClientNum)
                    {
                        if(m_BroadCastRecvBuf.iSendCount > m_ServerData.iSendCount)
                        {
                            bUpdateServer = true;
                        }
                        else if(m_BroadCastRecvBuf.iSendCount == m_ServerData.iSendCount)
                        {
                            if(m_BroadCastRecvAddr.sin_addr.s_addr < m_Server.s_addr) bUpdateServer = true;
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
            if(m_BroadCastRecvBuf.iSendCount > m_iSendCount)
            {
                bUpdateServer = true;
            }
            else if(m_BroadCastRecvBuf.iSendCount == m_iSendCount)
            {
                int         rc;
                DWORD       dwBytes;
                SOCKADDR_IN LocalIf;
                if(WSAIoctl( m_soBroadCast
                           , SIO_ROUTING_INTERFACE_QUERY
                           , &m_BroadCastRecvAddr
                           , sizeof(m_BroadCastRecvAddr)
                           , (SOCKADDR *)&LocalIf
                           , sizeof(LocalIf)
                           , &dwBytes
                           , NULL
                           , NULL ) == SOCKET_ERROR)
                {
                    Stop();
                    return 0;
                }
                if(m_BroadCastRecvAddr.sin_addr.s_addr < LocalIf.sin_addr.s_addr) bUpdateServer = true;
            }
        }
    }

    if(bUpdateServer)
    {
        m_Server        = m_BroadCastRecvAddr.sin_addr;
        m_ServerData    = m_BroadCastRecvBuf;
        m_bRecvServer   = true;
    }

    return 1;
}

int CNetworkSystem::OnAccept(float time)
{
    PRINT("OnAccept!");

    DWORD dwBytes, dwFlags;

    bool bSuccess = false;

    if(WSAGetOverlappedResult(m_CoClientData[m_iCoClinetAccept].s, &m_Overlapped[EEVENT_ACCEPT], &dwBytes, false, &dwFlags) == TRUE)
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
                if(PostHostRecv(m_iCoClinetAccept) == NO_ERROR)
                {
                    if(m_eState == ESTATE_WAITING)
                    {
                        m_eState = ESTATE_PAIRED;
                        m_bHost = true;
                    }
                    bSuccess = true;
                    m_iCoClientNum++;
                }
            }
        }
    }

    if(!bSuccess) SAFE_CLOSESOCKET(m_CoClientData[m_iCoClinetAccept].s);

    if(PostAccept() != NO_ERROR) return 0;

    return 1;
}

int CNetworkSystem::OnConnect(float time)
{
    PRINT("OnConnect!");

    DWORD dwBytes, dwFlags;

    if(WSAGetOverlappedResult(m_soClient, &m_Overlapped[EEVENT_CONNECT], &dwBytes, false, &dwFlags) == TRUE)
    {
        int opval = 1;
        if(setsockopt(m_soClient, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, (char*)&opval, sizeof(opval)) == SOCKET_ERROR)
        {
            if(StopClient(time) != NO_ERROR) return 0;
        }
        m_eState = ESTATE_PAIRED;
        m_bHost = false;
        if(PostClientRecv() != NO_ERROR)
        {
            if(StopClient(time) != NO_ERROR) return 0;
        }
    }
    else
    {
        if(StopClient(time) != NO_ERROR) return 0;
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

int CNetworkSystem::StopClient(float time)
{
    int rc;
    SAFE_CLOSESOCKET(m_soClient);
    m_eState = ESTATE_WAITING;
    rc = StartCheckServer(time);
    if(rc != NO_ERROR) return rc;
    return NO_ERROR;
}

int CNetworkSystem::StartCheckServer(float time)
{
    int rc;

    m_fCheckServerTime    = time;
    m_bCheckServer        = true;
    m_bRecvServer         = false;

    rc = PostBroadCastSend(time);
    if(rc != NO_ERROR) return rc;

    rc = PostBroadCastRecv();
    if(rc != NO_ERROR) return rc;

    return NO_ERROR;
}

void CNetworkSystem::StopCheckServer()
{
    m_bCheckServer = false;
}

void CNetworkSystem::CheckBroadCastServer(float time)
{
    if(!m_bCheckServer)                                               return;
    if(time - m_fCheckServerTime < m_cBroadCastCheckServerInterval)   return;

    m_fCheckServerTime = time;

    if(m_eState == ESTATE_WAITING)
    {
        if(m_bRecvServer)
        {
            SAFE_CLOSESOCKET(m_soListener);
            if(PostConnect() == NO_ERROR)
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
        if(PostBroadCastSend(time) != NO_ERROR) return 0;
    }

    return 1;
}

bool CNetworkSystem::FindEmptyCoClient(int& empty)
{
    for(int i = 0; i < m_iCoClientMax; i++)
    {
        if(m_CoClientData[i].s == INVALID_SOCKET)
        {
            empty = i;
            return true;
        }
    }
    return false;
}

void CNetworkSystem::ReleaseInternal()
{
    SAFE_CLOSESOCKET(m_soBroadCast);
    SAFE_CLOSESOCKET(m_soListener);
    SAFE_CLOSESOCKET(m_soClient);
    for(int i = 0; i < COCLIENT_MAX; i++)
    {
        ReleaseCoClientData(i);
    }
}

void CNetworkSystem::ReleaseCoClientData(int client)
{
    if(client >= COCLIENT_MAX) return;

    SAFE_CLOSESOCKET(m_CoClientData[client].s);
    m_CoClientData[client].bSending = false;
    while(m_CoClientData[client].SendBufQue.size())
    {
        GameData* pData = m_CoClientData[client].SendBufQue.front();
        SAFE_DELETE(pData);
        m_CoClientData[client].SendBufQue.pop_front();
    }
}