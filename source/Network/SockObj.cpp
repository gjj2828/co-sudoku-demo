//**************************************************
//File: SockObj.cpp
//Author: GaoJiongjiong
//Function: SocketµÄ·â×°
//**************************************************

#include "StdAfx.h"
#include "SockObj.h"
#include <MSTcpIP.h>

TSockObj::TSockObj(int id, INetworkEventManager* event_manager)
 : m_iId(id)
 , m_soMain(INVALID_SOCKET)
 , m_pEventManager(event_manager)
 , m_pBuf(NULL)
 , m_iBufLen(0)
 , m_bSending(false)
 , m_bRecving(false)
{
    for(int i = 0; i < EEVENT_MAX; i++)
    {
        m_Overlapped[i].hEvent = WSACreateEvent();
        m_hEvents[i] = m_Overlapped[i].hEvent;
    }
    m_pEventFunc[EEVENT_ACCEPT]     = &TSockObj::OnAccept;
    m_pEventFunc[EEVENT_CONNECT]    = &TSockObj::OnConnect;
    m_pEventFunc[EEVENT_SEND]       = &TSockObj::OnSend;
    m_pEventFunc[EEVENT_RECV]       = &TSockObj::OnRecv;
}

int TSockObj::Send(Packet* packet, SOCKADDR* addr, int namelen)
{
    SendData* pData = new SendData;

    pData->packet = (Packet*)malloc(packet->size);
    memcpy(pData->packet, packet, packet->size);
    if(addr != NULL)
    {
        memcpy(&pData->addr, addr, sizeof(SOCKADDR));
        pData->namelen = namelen;
    }

    m_queSendData.push_back(pData);

    return PostSend();
}

int TSockObj::Update()
{
    int rc;

    rc = WaitForMultipleObjects(EEVENT_MAX, m_hEvents, false, 0);

    if(rc == WAIT_TIMEOUT) return NO_ERROR;
    if(rc == WAIT_FAILED) return rc;

    for(int i = 0; i < EEVENT_MAX; i++)
    {
        rc = WaitForSingleObject(m_hEvents[i], 0);
        if(rc == WAIT_FAILED) return rc;
        if(rc == WAIT_OBJECT_0)
        {
            rc = (this->*m_pEventFunc[i])();
            if(rc != NO_ERROR) return rc;
        }
    }

    return NO_ERROR;
}

void TSockObj::Close()
{
    SAFE_CLOSESOCKET(m_soMain);

    m_pEventManager = NULL;
    m_iBufLen       = 0;
    m_bSending      = false;
    m_bRecving      = false;

    SAFE_FREE(m_pBuf);

    for(UINT i = 0; i < m_queSendData.size(); i++)
    {
        SAFE_DELETE(m_queSendData[i]);
    }
    m_queSendData.clear();

    for(int i = 0; i < EEVENT_MAX; i++)
    {
        WSAResetEvent(m_hEvents[i]);
    }
}

void TSockObj::Release()
{
    Close();
    delete this;
}

int TSockObj::PostSend()
{
    if(m_bSending) return NO_ERROR;

    int rc;

    while(m_queSendData.size() > 0)
    {
        SendData* pData = m_queSendData.front();

        rc = PostSendData(pData);

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

int TSockObj::PostEvent( INetworkEventManager::EEvent event, int ret, SOCKADDR* local, SOCKADDR* remote
                        , Packet* packet, char* buf, int buf_len, SOCKET sock )
{
	INetworkEventManager::Event evt;

	evt.pSockObj	= this;
	evt.eEvent		= event;
    evt.iRetCode	= ret;
	evt.pLocalAddr	= local;
    evt.pRemoteAddr = remote;
	evt.pPacket		= packet;
    evt.soAccept	= sock;
    evt.pBuf		= buf;
    evt.iBufLen     = buf_len;

	return m_pEventManager->HandleEvent(evt);
}

CTcpSockObj::CTcpSockObj(int id, INetworkEventManager* event_manager)
 : TSockObj(id, event_manager)
 , m_lpfnAcceptEx(NULL)
 , m_lpfnGetAcceptExSockaddrs(NULL)
 , m_soAccept(INVALID_SOCKET)
 , m_eRecvStep(ERECVSTEP_MIN)
 , m_iOffset(0)
 , m_iSize(0)
 , m_pRecvPacket(NULL)
{
}

int CTcpSockObj::Listen(SOCKADDR* addr, int namelen, int buf_len, int backlog, bool is_broadcast)
{
    if(m_soMain != INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_SOCKETALREADEXIST, -1); 

    DWORD dwBytes;
    int rc;

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    m_soMain = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_soMain == INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_CREATEFAIL, -1);

    if(bind(m_soMain, addr, namelen) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_BINDFAIL, WSAGetLastError());

    if(listen(m_soMain, backlog) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_LISTENFAIL, WSAGetLastError());

    if(WSAIoctl( m_soMain
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidAcceptEx
        , sizeof(guidAcceptEx)
        , &m_lpfnAcceptEx
        , sizeof(m_lpfnAcceptEx)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_LISTENFAIL, WSAGetLastError());

    if(WSAIoctl( m_soMain
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidGetAcceptExSockaddrs
        , sizeof(guidGetAcceptExSockaddrs)
        , &m_lpfnGetAcceptExSockaddrs
        , sizeof(m_lpfnGetAcceptExSockaddrs)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_LISTENFAIL, WSAGetLastError());

    rc = SetKeepAlive();
    if(rc != NO_ERROR) return PostEvent(INetworkEventManager::EEVENT_SETKEEPALIVETFAIL, rc);

    m_iBufLen = buf_len;
    return PostAccept();
}

int CTcpSockObj::Accept(SOCKET sock)
{
    if(sock == INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_INVALIDSOCKET, -1);
    m_soMain = sock;
    return PostRecv();
}

int CTcpSockObj::Connect(SOCKADDR* remote_addr, int remote_namelen, SOCKADDR* local_addr, int local_namelen, char* buf, int len)
{
    int             rc;
    DWORD           dwBytes;
    LPFN_CONNECTEX  lpfnConnectEx;

    GUID guidConnectEx = WSAID_CONNECTEX;

    if(m_pBuf) return PostEvent(INetworkEventManager::EEVENT_POSTCONNECTFAIL, -1);

    if(m_soMain != INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_SOCKETALREADEXIST, -1); 

    m_soMain = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_soMain == INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_CREATEFAIL, -1);

    if(bind(m_soMain, local_addr, local_namelen) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_BINDFAIL, WSAGetLastError());

    if( WSAIoctl( m_soMain
        , SIO_GET_EXTENSION_FUNCTION_POINTER
        , &guidConnectEx
        , sizeof(guidConnectEx)
        , &lpfnConnectEx
        , sizeof(lpfnConnectEx)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR ) return PostEvent(INetworkEventManager::EEVENT_POSTCONNECTFAIL, WSAGetLastError());

    rc = SetKeepAlive();
    if(rc != NO_ERROR) return PostEvent(INetworkEventManager::EEVENT_SETKEEPALIVETFAIL, rc);

    m_iBufLen = len;
    m_pBuf = (char*)malloc(m_iBufLen);
    memcpy(m_pBuf, buf, m_iBufLen);

    if(lpfnConnectEx(m_soMain, remote_addr, remote_namelen, m_pBuf, m_iBufLen, &dwBytes, &m_Overlapped[EEVENT_CONNECT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTCONNECTFAIL, rc);
    }

    return NO_ERROR;
}

void CTcpSockObj::Close()
{
    m_lpfnAcceptEx              = NULL;
    m_lpfnGetAcceptExSockaddrs  = NULL;
    m_eRecvStep                 = ERECVSTEP_MIN;
    m_iOffset                   = 0;
    m_iSize                     = 0;

    SAFE_CLOSESOCKET(m_soAccept);
    SAFE_DELETE(m_pRecvPacket);

    TSockObj::Close();
}

int CTcpSockObj::PostAccept()
{
    int rc;
    DWORD dwBytes;

    if(!m_lpfnAcceptEx || !m_lpfnGetAcceptExSockaddrs)  return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, -1);
    if(m_soAccept != INVALID_SOCKET)                    return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, -1);
    if(m_pBuf)                                          return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, -1);

    m_soAccept = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_soAccept == INVALID_SOCKET)                    return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, -1);

    m_pBuf    = (char*)malloc(m_iBufLen + (sizeof(SOCKADDR) + 16) * 2);

    if(m_lpfnAcceptEx(m_soMain, m_soAccept, m_pBuf, m_iBufLen, sizeof(SOCKADDR) + 16, sizeof(SOCKADDR) + 16, &dwBytes, &m_Overlapped[EEVENT_ACCEPT]) == FALSE)
    {
        rc = WSAGetLastError();
        if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTACCEPTFAIL, rc);
    }

    return NO_ERROR;
}

int CTcpSockObj::OnAccept()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_ACCEPT]);

    if(WSAGetOverlappedResult(m_soMain, &m_Overlapped[EEVENT_ACCEPT], &dwBytes, false, &dwFlags) == TRUE)
    {
        SOCKADDR*   pLocalAddr;
        SOCKADDR*   pRemoteAddr;
        int         iLocalAddrLen;
        int         iRemoteAddrLen;
        m_lpfnGetAcceptExSockaddrs( m_pBuf, m_iBufLen, sizeof(SOCKADDR) + 16, sizeof(SOCKADDR) + 16
            , (SOCKADDR **)&pLocalAddr, &iLocalAddrLen, (SOCKADDR **)&pRemoteAddr, &iRemoteAddrLen );
        rc = PostEvent(INetworkEventManager::EEVENT_ONACCEPT, NO_ERROR, pLocalAddr, pRemoteAddr, NULL, m_pBuf, m_iBufLen, m_soAccept);
    }
    else
    {
        rc = PostEvent(INetworkEventManager::EEVENT_ONACCEPTFAIL, WSAGetLastError());
    }

    if(rc != NO_ERROR) return rc;

    m_soAccept = INVALID_SOCKET;
    SAFE_FREE(m_pBuf);

    return PostAccept();
}

int CTcpSockObj::OnConnect()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_CONNECT]);

    if(WSAGetOverlappedResult(m_soMain, &m_Overlapped[EEVENT_CONNECT], &dwBytes, false, &dwFlags) == FALSE) return PostEvent(INetworkEventManager::EEVENT_ONCONNECTFAIL, WSAGetLastError());

    int opval = 1;
    if(setsockopt(m_soMain, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, (char*)&opval, sizeof(opval)) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_ONCONNECTFAIL, WSAGetLastError());

    rc = PostEvent(INetworkEventManager::EEVENT_ONCONNECT, NO_ERROR);
    if(rc != NO_ERROR) return rc;

    SAFE_FREE(m_pBuf);

    return PostRecv();
}

int CTcpSockObj::OnSend()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_SEND]);

    m_bSending = false;

    rc = WSAGetOverlappedResult(m_soMain, &m_Overlapped[EEVENT_SEND], &dwBytes, false, &dwFlags);

    if(rc == FALSE) return PostEvent(INetworkEventManager::EEVENT_ONSENDFAIL, WSAGetLastError());

    SendData* pData = m_queSendData.front();
    PostEvent(INetworkEventManager::EEVENT_ONSEND, NO_ERROR, NULL, &pData->addr, pData->packet);
    m_queSendData.pop_front();
    SAFE_DELETE(pData);

    return PostSend();
}

int CTcpSockObj::OnRecv()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_RECV]);

    m_bRecving = false;

    rc = WSAGetOverlappedResult(m_soMain, &m_Overlapped[EEVENT_RECV], &dwBytes, false, &dwFlags);

    if(rc == FALSE) return PostEvent(INetworkEventManager::EEVENT_ONRECVFAIL, WSAGetLastError());
    rc = OnRecv(dwBytes);
    if(rc != NO_ERROR) return rc;

    return PostRecv();
}

int CTcpSockObj::PostSendData(SendData* data_ptr)
{
    WSABUF  buf;
    DWORD   dwBytes;

    buf.buf = (char*)data_ptr->packet;
    buf.len = data_ptr->packet->size;

    return WSASend(m_soMain, &buf, 1, &dwBytes, 0, &m_Overlapped[EEVENT_SEND], NULL);
}

int CTcpSockObj::PostRecv()
{
    WSABUF  buf;
    DWORD   dwBytes, dwFlags;

    int rc = NO_ERROR;

    if(m_bRecving) return NO_ERROR;

    m_bRecving = true;

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
        rc = WSARecv(m_soMain, &buf, 1, &dwBytes, &dwFlags, &m_Overlapped[EEVENT_RECV], NULL);
        break;

        if(rc == SOCKET_ERROR)
        {
            rc = WSAGetLastError();
            if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTRECVFAIL, rc);;
            return NO_ERROR;
        }
        WSAResetEvent(m_Overlapped[EEVENT_SEND].hEvent);

        rc = OnRecv(dwBytes);
        if(rc != NO_ERROR) return rc;
    }

    return NO_ERROR;
}

int CTcpSockObj::OnRecv(int bytes)
{
    if(bytes == 0) return PostEvent(INetworkEventManager::EEVENT_CLOSE, NO_ERROR);
    m_iOffset += bytes;
    switch(m_eRecvStep)
    {
    case ERECVSTEP_SIZE:
        assert(m_iOffset <= sizeof(psize_t));
        if(m_iOffset == sizeof(psize_t))
        {
            if(m_pRecvPacket) return PostEvent(INetworkEventManager::EEVENT_ONRECVFAIL, -1);
            m_pRecvPacket = (Packet*)malloc(m_iSize);
            m_pRecvPacket->size = m_iSize;
            m_eRecvStep = ERECVSTEP_PACKET;
        }
        break;
    case ERECVSTEP_PACKET:
        assert(m_iOffset <= m_pRecvPacket->size);
        if(m_iOffset == m_pRecvPacket->size)
        {
            PostEvent(INetworkEventManager::EEVENT_ONRECV, NO_ERROR, NULL, NULL, m_pRecvPacket);
            SAFE_DELETE(m_pRecvPacket);
            m_iOffset   = 0;
            m_iSize     = 0;
            m_eRecvStep = ERECVSTEP_MIN;
        }
        break;
    }

    return NO_ERROR;
}

int CTcpSockObj::SetKeepAlive()
{
    DWORD dwBytes;

    BOOL keepalive = TRUE;
    if(setsockopt(m_soMain, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepalive, sizeof(keepalive)) == SOCKET_ERROR) return WSAGetLastError();

    tcp_keepalive klin = {0};
    tcp_keepalive klout = {0};
    klin.onoff              = 1;
    klin.keepalivetime      = 500;
    klin.keepaliveinterval  = 100;
    dwBytes                 = 0;
    if(WSAIoctl(m_soMain, SIO_KEEPALIVE_VALS, &klin, sizeof(klin), &klout, sizeof(klout), &dwBytes, NULL, NULL) == SOCKET_ERROR) return WSAGetLastError();

    return NO_ERROR;
}

CUdpSockObj::CUdpSockObj(int id, INetworkEventManager* event_manager) : TSockObj(id, event_manager)
{
}

int CUdpSockObj::Listen(SOCKADDR* addr, int namelen, int buf_len, int backlog, bool is_broadcast)
{
    if(m_soMain != INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_SOCKETALREADEXIST, -1); 

    m_soMain = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(m_soMain == INVALID_SOCKET) return PostEvent(INetworkEventManager::EEVENT_CREATEFAIL, -1);
    BOOL bVal = TRUE;
    if(bind(m_soMain, addr, namelen) == SOCKET_ERROR) return PostEvent(INetworkEventManager::EEVENT_BINDFAIL, WSAGetLastError());
    if(is_broadcast)
    {
        if(setsockopt(m_soMain, SOL_SOCKET, SO_BROADCAST, (char*)&bVal, sizeof(BOOL)) == SOCKET_ERROR)
            return PostEvent(INetworkEventManager::EEVENT_SETBROADCASTFAIL, WSAGetLastError());
    }
    m_iBufLen = buf_len;
    m_pBuf = (char*)malloc(m_iBufLen);
    return PostRecv();
}

int CUdpSockObj::OnSend()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_SEND]);

    m_bSending = false;

    rc = WSAGetOverlappedResult(m_soMain, &m_Overlapped[EEVENT_SEND], &dwBytes, false, &dwFlags);

    if(rc == TRUE)
    {
        SendData* pData = m_queSendData.front();
        PostEvent(INetworkEventManager::EEVENT_ONSEND, NO_ERROR, NULL, &pData->addr, pData->packet);
        m_queSendData.pop_front();
        SAFE_DELETE(pData);
    }
    else
    {
        PostEvent(INetworkEventManager::EEVENT_ONSENDFAIL, WSAGetLastError());
    }

    return PostSend();
}

int CUdpSockObj::OnRecv()
{
    int rc;
    DWORD dwBytes, dwFlags;

    WSAResetEvent(m_hEvents[EEVENT_RECV]);

    m_bRecving = false;

    rc = WSAGetOverlappedResult(m_soMain, &m_Overlapped[EEVENT_RECV], &dwBytes, false, &dwFlags);

    if(rc == FALSE) PostEvent(INetworkEventManager::EEVENT_ONRECVFAIL, WSAGetLastError());
    else OnRecv(dwBytes);

    return PostRecv();
}

int CUdpSockObj::PostSendData(SendData* data_ptr)
{
    WSABUF  buf;
    DWORD   dwBytes;

    buf.buf = (char*)data_ptr->packet;
    buf.len = data_ptr->packet->size;

    return WSASendTo(m_soMain, &buf, 1, &dwBytes, 0, &data_ptr->addr, data_ptr->namelen, &m_Overlapped[EEVENT_SEND], NULL);
}

int CUdpSockObj::PostRecv()
{
    WSABUF  buf;
    DWORD   dwBytes, dwFlags;

    int rc = NO_ERROR;

    if(m_bRecving) return NO_ERROR;

    m_bRecving = true;

    while(1)
    {
        m_iRecvAddrSize = sizeof(m_RecvAddr);
        buf.buf = m_pBuf;
        buf.len = m_iBufLen;
        dwFlags = 0;
        rc = WSARecvFrom(m_soMain, &buf, 1, &dwBytes, &dwFlags, &m_RecvAddr, &m_iRecvAddrSize, &m_Overlapped[EEVENT_RECV], NULL);

        if(rc == SOCKET_ERROR)
        {
            rc = WSAGetLastError();
            if(rc != WSA_IO_PENDING) return PostEvent(INetworkEventManager::EEVENT_POSTRECVFAIL, rc);
            return NO_ERROR;
        }
        WSAResetEvent(m_Overlapped[EEVENT_SEND].hEvent);
        OnRecv(dwBytes);
    }

    return NO_ERROR;
}

int CUdpSockObj::OnRecv(int bytes)
{
    Packet* pPacket = (Packet*)m_pBuf;
    if(pPacket->size != bytes) return -1;
    DWORD       dwBytes;
    SOCKADDR    LocalIf;
    if(WSAIoctl( m_soMain
        , SIO_ROUTING_INTERFACE_QUERY
        , &m_RecvAddr
        , m_iRecvAddrSize
        , &LocalIf
        , sizeof(LocalIf)
        , &dwBytes
        , NULL
        , NULL ) == SOCKET_ERROR) return WSAGetLastError();
    PostEvent(INetworkEventManager::EEVENT_ONRECV, NO_ERROR, &LocalIf, &m_RecvAddr, pPacket);

    return NO_ERROR;
}
