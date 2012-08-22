#include "StdAfx.h"
#include "NetworkSystem.h"

#define ERROR_CHECK1(rc)                    \
{                                           \
    if((rc) == SOCKET_ERROR) return (rc);   \
}

#define ERROR_CHECK2(rc)                        \
{                                               \
    if((rc) == SOCKET_ERROR)                    \
    {                                           \
        (rc) = WSAGetLastError();               \
        if((rc) != WSA_IO_PENDING) return (rc); \
    }                                           \
}

#define ERROR_CHECK3(rc)                \
{                                       \
    if((rc) == NO_ERROR) return (rc);   \
}

#define ERROR_CHECK4(rc)                    \
{                                           \
    if((rc) == WAIT_FAILED) return (rc);    \
}

const char* CNetworkSystem::m_cBroadCastSendAddr = "192.168.255.255";

CNetworkSystem::CNetworkSystem()
: m_eState(ESTATE_STOPPED)
, m_iSendCount(0)
, m_bbcSending(false)
, m_fbcSendTime(0.0f)
, m_ibcRecAddrSize(0)
, m_soBroadCast(INVALID_SOCKET)
{
    ZeroMemory(m_hEvents, sizeof(HANDLE) * EEVENT_MAX);
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

int CNetworkSystem::Update(float time)
{
    if(m_eState == ESTATE_STOPPED) return -1;

    int rc;
    DWORD dwBytes, dwFlags;

    rc = WaitForMultipleObjects(EEVENT_MAX, m_hEvents, false, 0);
    ERROR_CHECK4(rc);

    if(rc != WAIT_TIMEOUT)
    {
        rc = WaitForSingleObject(m_hEvents[EEVENT_BROADCAST_SEND], 0);
        ERROR_CHECK4(rc);
        if(rc == WAIT_OBJECT_0)
        {
            WSAResetEvent(m_hEvents[EEVENT_BROADCAST_SEND]);
            rc = WSAGetOverlappedResult(m_soBroadCast, &m_Overlapped[EEVENT_BROADCAST_SEND], &dwBytes, false, &dwFlags);
            m_bbcSending = false;
        }
    }

    return NO_ERROR;
}

int CNetworkSystem::Start(EMode mode, float time)
{
    if(m_eState != ESTATE_STOPPED) return -1;

    int     rc;

    m_iSendCount    = 0;
    m_eState        = ESTATE_WAITING;

    for(int i = 0; i < EEVENT_MAX; i++)
    {
        WSAResetEvent(m_hEvents[i]);
    }

    m_soBroadCast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(m_soBroadCast == INVALID_SOCKET) return -1;

    rc = bind(m_soBroadCast, (SOCKADDR*)&m_bcBindAddr, sizeof(m_bcBindAddr));
    ERROR_CHECK1(rc);

    rc = BroadCastSend(time);
    ERROR_CHECK3(rc);

    rc = BroadCastRecv();
    ERROR_CHECK3(rc);

    return NO_ERROR;
}

void CNetworkSystem::Stop()
{
    m_eState = ESTATE_STOPPED;

    SAFE_CLOSESOCKET(m_soBroadCast);
}

int CNetworkSystem::BroadCastSend(float time)
{
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
    ERROR_CHECK2(rc);

    m_bbcSending = true;
    m_fbcSendTime = time;

    return NO_ERROR;
}

int CNetworkSystem::BroadCastRecv()
{
    WSABUF  buf;
    int     rc;
    DWORD   dwBytes, dwFlags;

    buf.buf = (char*)&m_bcRecvBuf;
    buf.len = sizeof(m_bcRecvBuf);
    dwFlags = 0;
    rc = WSARecvFrom(m_soBroadCast, &buf, 1, &dwBytes, &dwFlags, (SOCKADDR*)&m_bcRecvAddr, &m_ibcRecAddrSize, &m_Overlapped[EEVENT_BROADCAST_RECV], NULL);
    ERROR_CHECK2(rc);

    return NO_ERROR;
}