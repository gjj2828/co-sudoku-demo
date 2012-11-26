#include "StdAfx.h"
#include "SockObj.h"

CSockObj::CSockObj(int id, ESockType type, INetworkEventManager* event_manager)
 : m_iId(id)
 , m_eSockType(ESOCKTYPE_UNDECIDED)
 , m_Sock(INVALID_SOCKET)
 , m_lpfnAcceptEx(NULL)
 , m_lpfnGetAcceptExSockaddrs(NULL)
 , m_pEventManager(event_manager)
 , m_pAcceptBuf(NULL)
 , m_pAcceptBufOrg(NULL)
 , m_iAcceptBufLen(0)
 , m_bSending(false)
 , m_bRecving(false)
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
    m_pEventFunc[EEVENT_ACCEPT] = CSockObj::OnAccept;
    m_pEventFunc[EEVENT_CONNECT] = CSockObj::OnConnect;
    m_pEventFunc[EEVENT_SEND] = CSockObj::OnSend;
    m_pEventFunc[EEVENT_RECV] = CSockObj::OnRecv;
}

CSockObj::~CSockObj()
{
    Close();
}

int CSockObj::Create(ESockType type)
{
    if(m_eSockType != ESOCKTYPE_UNDECIDED) return PostEvent(INetworkEventManager::EEVENT_CREATEFAIL, -1);

    switch(type)
    {
    case ESOCKTYPE_TCP:
        m_Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(m_Sock == INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_CREATEFAIL, -1);
        break;
    case ESOCKTYPE_UDP:
        m_Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(m_Sock == INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_CREATEFAIL, -1);
        break;
    default:
        assert(0);
        return PostEvent(INetworkEventManager::EEVENT_CREATEFAIL, -1);
    }
    m_eSockType = type;

    return NO_ERROR;
}

void CSockObj::Close()
{
    SAFE_CLOSESOCKET(m_Sock);

    m_eSockType                 = ESOCKTYPE_UNDECIDED;
    m_lpfnAcceptEx              = NULL;
    m_lpfnGetAcceptExSockaddrs  = NULL;
    m_pEventManager             = NULL;
    m_pAcceptBufOrg             = NULL;
    m_iAcceptBufLen             = 0;
    m_bSending                  = false;
    m_bRecving                  = false;
    m_eRecvStep                 = ERECVSTEP_MIN;
    m_iOffset                   = 0;
    m_iSize                     = 0;

    SAFE_FREE(m_pAcceptBuf);

    for(int i = 0; i < m_queSendData.size(); i++)
    {
        SAFE_DELETE(m_queSendData[i]);
    }
    m_queSendData.clear();

    SAFE_DELETE(m_pRecvPacket);

    for(int i = 0; i < EEVENT_MAX; i++)
    {
        WSAResetEvent(m_hEvents[i]);
    }
}

int CSockObj::Bind(SOCKADDR* addr, int namelen)
{
    if(bind(m_Sock, addr, namelen) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_BINDFAIL, WSAGetLastError());
    return NO_ERROR;
}

int CSockObj::Listen(int backlog)
{
    DWORD dwBytes;

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    if(m_eSockType != ESOCKTYPE_TCP) return PostEvent(INetworkEventManager::EEVENT_LISTENFAIL, -1);

    if(listen(m_Sock, backlog) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_LISTENFAIL, WSAGetLastError());

    if(WSAIoctl( m_Sock
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidAcceptEx
        , sizeof(guidAcceptEx)
        , &m_lpfnAcceptEx
        , sizeof(m_lpfnAcceptEx)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_LISTENFAIL, WSAGetLastError());

    if(WSAIoctl( m_Sock
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidGetAcceptExSockaddrs
        , sizeof(guidGetAcceptExSockaddrs)
        , &m_lpfnGetAcceptExSockaddrs
        , sizeof(m_lpfnGetAcceptExSockaddrs)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_LISTENFAIL, WSAGetLastError());

    return NO_ERROR;
}

int CSockObj::PostAccept(ISockObj* accept, char* buf, int len)
{
    int rc;
    DWORD dwBytes;

    if(m_eSockType != ESOCKTYPE_TCP)                    return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, -1);
    if(!m_lpfnAcceptEx || !m_lpfnGetAcceptExSockaddrs)  return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, -1);
    if(!m_pAcceptBuf)                                   return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, -1);

    m_pAcceptBufOrg   = buf;
    m_iAcceptBufLen   = len;
    m_pAcceptBuf      = malloc(m_iAcceptBufLen + (sizeof(SOCKADDR) + 16) * 2);

    if(m_lpfnAcceptEx(m_Sock, accept->GetSocket(), m_pAcceptBuf, m_iAcceptBufLen, sizeof(SOCKADDR) + 16, sizeof(SOCKADDR) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, rc);
    }

    return NO_ERROR;
}

int CSockObj::PostConnect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len)
{
    int             rc;
    DWORD           dwBytes;
    LPFN_CONNECTEX  lpfnConnectEx;

    GUID guidConnectEx = WSAID_CONNECTEX;

    if(m_eSockType != ESOCKTYPE_TCP) return PostEvent(INetworkEventManager::EEVENT_POSTCONNECTFAIL, -1);

    if(bind(m_Sock, local_addr, local_namelen) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_POSTCONNECTFAIL, WSAGetLastError());

    if( WSAIoctl( m_Sock
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidConnectEx
        , sizeof(guidConnectEx)
        , &lpfnConnectEx
        , sizeof(lpfnConnectEx)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_POSTCONNECTFAIL, WSAGetLastError());

    if(lpfnConnectEx(m_Sock, &remote_addr, remote_namelen, buf, len, &dwBytes, &m_Overlapped[EEVENT_CONNECT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTCONNECTFAIL, rc);
    }

    return NO_ERROR;
}

int CSockObj::PostSend(Packet* packet, SOCKADDR* addr, int namelen)
{
    SendData* pData = new SendData;

    pData->packet = malloc(packet->size);
    memcpy(pData->packet, packet, packet->size);
    memcpy(&pData->addr, addr, sizeof(SOCKADDR));
    pData->namelen = namelen;
    
    m_queSendData.push_back(pData);

    return PostSend();
}

int CSockObj::PostRecv()
{
    WSABUF  buf;
    DWORD   dwBytes, dwFlags;

    int rc = NO_ERROR;

    if(!m_bRecving) return NO_ERROR;

    m_bRecving = true;

    switch(m_eSockType)
    {
    case ESOCKTYPE_TCP:
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
            break;

            if(rc == SOCKET_ERROR)
            {
                rc = WSAGetLastError();
                if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTRECVFAIL, rc);;
                return NO_ERROR;
            }
            WSAResetEvent(m_Overlapped[EEVENT_SEND].hEvent);

            if(dwBytes == 0) return PostEvent(INetworkEventManager::EEVENT_CLOSE, NO_ERROR);

            rc = OnRecv(dwBytes);
            if(rc != NO_ERROR) return rc;
        }
        break;
    case ESOCKTYPE_UDP:
        while(1)
        {
            m_iRecvAddrSize = sizeof(m_RecvAddr);
            rc = WSARecvFrom(m_Sock, &buf, 1, &dwBytes, &dwFlags, &m_RecvAddr, &m_iRecvAddrSize, &m_Overlapped[EEVENT_RECV], NULL);

            if(rc == SOCKET_ERROR)
            {
                rc = WSAGetLastError();
                if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTRECVFAIL, rc);
                return NO_ERROR;
            }
            WSAResetEvent(m_Overlapped[EEVENT_SEND].hEvent);
        }
        break;
    default:
        assert(0);
        break;
    }

    return NO_ERROR;
}

int CSockObj::Update()
{
    int rc;

    rc = WaitForMultipleObjects(EEVENT_MAX, m_hEvents, false, 0);

    if(rc == WAIT_TIMEOUT) return NO_ERROR;
    if(rc == FAILED) return rc;

	if(rc != WAIT_TIMEOUT)
	{
        for(int i = 0; i < EEVENT_MAX; i++)
        {
            rc = WaitForSingleObject(m_hEvents[i], 0);
            if(rc == FAILED) return rc;
            if(rc == WAIT_OBJECT_0)
            {
                rc = this->m_pEventFunc[i]();
                if(rc != NO_ERROR) return rc;
            }
        }
	}
}

int CSockObj::PostSend()
{
    if(m_eSockType == ESOCKTYPE_UNDECIDED) return PostEvent(INetworkEventManager::EEVENT_POSTSENDFAIL, -1);
    if(m_bSending) return NO_ERROR;

    WSABUF  buf;
    int     rc;
    DWORD   dwBytes;

    while(m_queSendData.size() > 0)
    {
        SendData* pData = m_queSendData.front();

        buf.buf = pData->packet;
        buf.len = pData->packet->size;

        switch(m_eSockType)
        {
        case ESOCKTYPE_TCP:
            rc = WSASend(m_Sock, &buf, 1, &dwBytes, 0, &m_Overlapped[EEVENT_SEND], NULL);
            break;
        case ESOCKTYPE_UDP:
            rc = WSASendTo(m_Sock, &buf, 1, &dwBytes, 0, &pData->addr, pData->namelen, &m_Overlapped[EEVENT_SEND], NULL);
            break;
        default:
            assert(0);
            return PostEvent(INetworkEventManager::EEVENT_POSTSENDFAIL, -1);
        }

        if(rc == SOCKET_ERROR)
        {
            rc = WSAGetLastError();
            if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTSENDFAIL, rc);
            break;
        }

        WSAResetEvent(m_Overlapped[EEVENT_SEND].hEvent);
        m_queSendData.pop_front();
        SAFE_DELETE(pData);
    }

    if(m_queSendData.size() > 0) m_bSending = true;

    return NO_ERROR;
}

int CSockObj::PostEvent(INetworkEventManager::EEvent event, int ret, SOCKADDR* local, SOCKADDR* remote, Packet* recv)
{
	INetworkEventManager::Event evt;

	evt.pSockObj	= this;
	evt.eEvent		= event;
	evt.pLocalAddr	= local;
    evt.pRemoteAddr	= remote;
	evt.pRecv		= recv;
	evt.iRetCode	= ret;

	return m_pEventManager->HandleEvent(evt);
}

int CSockObj::OnAccept()
{
	int rc;
	DWORD dwBytes, dwFlags;

	WSAResetEvent(m_hEvents[EEVENT_ACCEPT]);

	if(WSAGetOverlappedResult(m_Sock, &m_Overlapped[EEVENT_ACCEPT], &dwBytes, false, &dwFlags) == TRUE)
	{
        if(m_iAcceptBufLen > 0)
        {
            memcpy(m_pAcceptBufOrg, m_pAcceptBuf, m_iAcceptBufLen);
        }
        SOCKADDR*   pLocalAddr;
        SOCKADDR*   pRemoteAddr;
        int         iLocalAddrLen;
        int         iRemoteAddrLen;
        m_lpfnGetAcceptExSockaddrs( m_pAcceptBuf, sizeof(m_pAcceptBuf) - (sizeof(SOCKADDR) + 16) * 2, sizeof(SOCKADDR) + 16, sizeof(SOCKADDR) + 16
                                  , (SOCKADDR **)&pLocalAddr, &iLocalAddrLen, (SOCKADDR **)&pRemoteAddr, &iRemoteAddrLen );
        rc = PostEvent(INetworkEventManager::EEVENT_ACCEPT, NO_ERROR, pLocalAddr, pRemoteAddr);
	}
	else
	{
		rc = PostEvent(INetworkEventManager::EEVENT_ONACCEPTFAIL, WSAGetLastError());
	}

    SAFE_FREE(m_pAcceptBuf);

    return rc;
}

int CSockObj::OnConnect()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_CONNECT]);

    if(WSAGetOverlappedResult(m_Sock, &m_Overlapped[EEVENT_CONNECT], &dwBytes, false, &dwFlags) == FALSE) return PostEvent(INetworkEventManager::EEVENT_ONACCEPTFAIL, WSAGetLastError());

    int opval = 1;
    if(setsockopt(m_Sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, (char*)&opval, sizeof(opval)) != SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_ONACCEPTFAIL, WSAGetLastError());

    return PostEvent(INetworkEventManager::EEVENT_CONNECT, NO_ERROR);
}

int CSockObj::OnSend()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_SEND]);

    m_bSending = false;
    m_queSendData.pop_front();
    SAFE_DELETE(pData);

    rc = WSAGetOverlappedResult(m_Sock, &m_Overlapped[EEVENT_SEND], &dwBytes, false, &dwFlags);

    switch(m_eSockType)
    {
    case ESOCKTYPE_TCP:
        if(rc == FALSE) return PostEvent(INetworkEventManager::EEVENT_ONSENDFAIL, WSAGetLastError());
        break;
    case ESOCKTYPE_UDP:
        if(rc == FALSE) PostEvent(INetworkEventManager::EEVENT_ONSENDFAIL, WSAGetLastError());
        break;
    default:
        assert(0);
        break;
    }

    return PostSend;
}

int CSockObj::OnRecv()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_RECV]);

    m_bRecving = false;

    rc = WSAGetOverlappedResult(m_Sock, &m_Overlapped[EEVENT_SEND], &dwBytes, false, &dwFlags);

    switch(m_eSockType)
    {
    case ESOCKTYPE_TCP:
        if(rc == FALSE) return PostEvent(INetworkEventManager::EEVENT_ONRECVFAIL, WSAGetLastError());
        rc = OnRecv(dwBytes);
        if(rc != NO_ERROR) return rc;
        break;
    case ESOCKTYPE_UDP:
        if(rc == FALSE) PostEvent(INetworkEventManager::EEVENT_ONRECVFAIL, WSAGetLastError());
        else PostEvent(INetworkEventManager::EEVENT_RECV, NO_ERROR, NULL, m_pRecvPacket);
        SAFE_DELETE(m_pRecvPacket);
        break;
    default:
        assert(0);
        break;
    }

    return PostRecv();
}

int CSockObj::OnRecv(int bytes)
{
    m_iOffset += bytes;
    switch(m_eRecvStep)
    {
    case ERECVSTEP_SIZE:
        assert(m_iOffset <= sizeof(psize_t));
        if(m_iOffset == sizeof(psize_t))
        {
            if(m_pRecvPacket) return PostEvent(INetworkEventManager::EEVENT_ONRECVFAIL, -1);
            m_pRecvPacket = malloc(m_iSize);
            m_pRecvPacket->size = m_iSize;
            m_eRecvStep = ERECVSTEP_PACKET;
        }
        break;
    case ERECVSTEP_PACKET:
        assert(m_iOffset <= m_pRecvPacket->size);
        if(m_iOffset == m_pRecvPacket->size)
        {
            PostEvent(INetworkEventManager::EEVENT_RECV, NO_ERROR, NULL, m_pRecvPacket);
            SAFE_DELETE(m_pRecvPacket);
            m_iOffset   = 0;
            m_iSize     = 0;
            m_eRecvStep = ERECVSTEP_MIN;
        }
        break;
    }

    return NO_ERROR;
}