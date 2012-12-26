#include "StdAfx.h"
#include "NetworkSystem.h"
#include "SockObj.h"

#define WAITFAILEDRTN(rc)   \
{                           \
    if((rc) == WAIT_FAILED) \
    {                       \
        Stop();             \
        return;             \
    }                       \
}

#define SAFE_DELETESOCKOBJ(p)   \
{                               \
    if(p) DeleteSockObj(p);     \
    (p) = NULL;                 \
}

#define STOP_RTN0   \
{                   \
    Stop();  \
    return 0;       \
}

#define STOP_RTN    \
{                   \
    Stop();  \
    return;         \
}

const char* CNetworkSystem::m_cBroadCastSendAddr            = "192.168.255.255";
const float CNetworkSystem::m_cBroadCastSendInterval        = 1.0f;
const float CNetworkSystem::m_cBroadCastCheckServerInterval = 1.0f;

CNetworkSystem::CNetworkSystem()
: m_eState(ESTATE_STOP)
, m_iSendCount(0)
, m_fBroadCastSendTime(0.0f)
, m_fRecvServerTime(0.0f)
, m_iCoClientMax(COCLIENT_MAX)
, m_iCoClientNum(0)
, m_bRecvServer(false)
, m_fTime(0.0f)
{
    ZeroMemory(m_pSockObjs, sizeof(ISockObj*) * ESOCKOBJTYPE_MAX);
}

int CNetworkSystem::Init()
{
    WSADATA wsd;

    if(WSAStartup(MAKEWORD(2, 2), &wsd)) ERROR_RTN0("Can\'t load Winsock!");

    m_BroadCastBindAddr.sin_family      = AF_INET;
    m_BroadCastBindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_BroadCastBindAddr.sin_port        = htons(BROADCAST_PORT);

    m_BroadCastSendAddr.sin_family      = AF_INET;
    m_BroadCastSendAddr.sin_addr.s_addr = inet_addr(m_cBroadCastSendAddr);
    m_BroadCastSendAddr.sin_port        = htons(BROADCAST_PORT);

    m_LocalBindAddr.sin_family          = AF_INET;
    m_LocalBindAddr.sin_addr.s_addr     = htonl(INADDR_ANY);
    m_LocalBindAddr.sin_port            = htons(MAIN_PORT);

    return 1;
}

void CNetworkSystem::Release()
{

    Stop();
    WSACleanup();
    this->~CNetworkSystem();
}

int CNetworkSystem::Start(EMode mode, float time)
{
    PRINT("Network start!\n");
    
    if(m_eState != ESTATE_STOP) return 0;

    m_fTime         = time;
    m_iSendCount    = 0;
    m_iCoClientNum  = 0;

    ChangeState(ESTATE_WAITING);

    m_pSockObjs[ESOCKOBJTYPE_LISTEN] = CreateSockObj(ESOCKOBJTYPE_LISTEN);
    if(m_pSockObjs[ESOCKOBJTYPE_LISTEN]->Listen(ISockObj::ESOCKTYPE_TCP, (SOCKADDR*)&m_LocalBindAddr
        , sizeof(m_LocalBindAddr), 5, GAMENAME_LEN) != NO_ERROR) STOP_RTN0;

    m_pSockObjs[ESOCKOBJTYPE_BROADCAST] = CreateSockObj(ESOCKOBJTYPE_BROADCAST);
    if(m_pSockObjs[ESOCKOBJTYPE_BROADCAST]->Listen(ISockObj::ESOCKTYPE_UDP, (SOCKADDR*)&m_LocalBindAddr
        , sizeof(m_LocalBindAddr)) != NO_ERROR) STOP_RTN0;

    return 1;
}

void CNetworkSystem::Stop()
{
    PRINT("Network stop!\n");
    ChangeState(ESTATE_STOP);
    for(int i = 0; i < ESOCKOBJTYPE_MAX; i++)
    {
        if(!m_pSockObjs[i]) continue;
        SAFE_DELETESOCKOBJ(m_pSockObjs[i]);
    }
    FreeDeletedSockObj();
    OnStop();
}

int CNetworkSystem::Update(float time)
{
    if(m_eState == ESTATE_STOP) return -1;

    int rc;

    m_fTime = time;

    for(int i = 0; i < ESOCKOBJTYPE_MAX; i++)
    {
        if(m_pSockObjs[i])
        {
            rc = m_pSockObjs[i]->Update();
            if(rc != NO_ERROR) break;
        }
    }

    if(rc == NO_ERROR)
    {
        BroadCastCheckInfo();
        rc = BroadCastSendInfo();
    }

    if(rc != NO_ERROR) Stop();

    FreeDeletedSockObj();

    return rc;
}

void CNetworkSystem::Send(Packet* packet)
{
    if(m_eState == ESTATE_HOST)
    {
        for(int i = 0; i < COCLIENT_MAX; i++)
        {
            Send(packet, i);
        }
    }
    else if(m_eState == ESTATE_CLIENT)
    {
        ISockObj* pSockObj = m_pSockObjs[ESOCKOBJTYPE_CLIENT];
        if(pSockObj) pSockObj->Send(packet);
    }
}

void CNetworkSystem::Send(Packet* packet, int client)
{
    ISockObj* pSockObj = m_pSockObjs[ESOCKOBJTYPE_COCLIENT_MIN +client];
    if(pSockObj) pSockObj->Send(packet);
}

void CNetworkSystem::RegisterListener(INetworkListener* listener)
{
    for(UINT i = 0; i < m_vectorListener.size(); i++)
    {
        if(m_vectorListener[i] == listener) return;
    }
    m_vectorListener.push_back(listener);
}

void CNetworkSystem::UnRegisterListener(INetworkListener* listener)
{
    for(ListenerVector::iterator it = m_vectorListener.begin(); it != m_vectorListener.end(); it++)
    {
        if(*it != listener) continue;
        m_vectorListener.erase(it);
        return;
    }
}

int CNetworkSystem::HandleEvent(const Event& event)
{
    switch(event.eEvent)
    {
    case EEVENT_ONACCEPT:
        if((m_eState == ESTATE_WAITING || (m_eState == ESTATE_HOST && m_iCoClientNum < m_iCoClientMax))
            && (strcmp(event.pBuf, GAME_NAME) == 0))
        {
            int empty;
            if(!FindEmptyCoClient(empty)) return -1;
            m_pSockObjs[ESOCKOBJTYPE_COCLIENT_MIN + empty] = CreateSockObj(empty);
            m_pSockObjs[ESOCKOBJTYPE_COCLIENT_MIN + empty]->Accept(event.soAccept);
            if(m_eState == ESTATE_WAITING) ChangeState(ESTATE_HOST);
            m_iCoClientNum++;
            OnAccept(empty);
        }
        else
        {
            if(event.soAccept != INVALID_SOCKET) closesocket(event.soAccept);
        }
        break;

    case  EEVENT_ONCONNECT:
        if(m_eState == ESTATE_CONNECTING)
        {
            ChangeState(ESTATE_CLIENT);
        }
        else
        {
            SAFE_DELETESOCKOBJ(m_pSockObjs[ESOCKOBJTYPE_CLIENT]);
        }
        break;

    case EEVENT_ONRECV:
        if(event.pSockObj->GetId() == ESOCKOBJTYPE_BROADCAST)
        {
            if(m_eState == ESTATE_WAITING && event.pPacket->type == EPACKETTYPE_BROADCAST)
            {
                BroadCastPacket* packet = (BroadCastPacket*)event.pPacket;
                if(strcmp(packet->sGameName, GAME_NAME) == 0)
                {
                    bool bCheck                 = false;
                    SOCKADDR_IN* pLocalAddr     = (SOCKADDR_IN*)event.pLocalAddr;
                    SOCKADDR_IN* pRemoteAddr    = (SOCKADDR_IN*)event.pRemoteAddr;
                    if(packet->iState == ESTATE_HOST)
                    {
                        if(packet->iCoClientNum < m_iCoClientMax) bCheck = true;
                    }
                    else if(packet->iState == ESTATE_WAITING)
                    {
                        if(packet->iSendCount > m_iSendCount)
                        {
                            bCheck = true;
                        }
                        else if(packet->iSendCount == m_iSendCount)
                        {
                            if(pRemoteAddr->sin_addr.s_addr < pLocalAddr->sin_addr.s_addr) bCheck = true;
                        }
                    }

                    if(bCheck && m_bRecvServer)
                    {
                        bCheck = false;
                        if(pRemoteAddr->sin_addr.s_addr == m_Server.s_addr)
                        {
                            bCheck = true;
                        }
                        else
                        {
                            if(packet->iState == ESTATE_HOST)
                            {
                                if(m_ServerPacket.iState == ESTATE_HOST)
                                {
                                    if(packet->iCoClientNum > m_ServerPacket.iCoClientNum)  bCheck = true;
                                    else if(packet->iSendCount > m_ServerPacket.iSendCount) bCheck = true;
                                    else if(pRemoteAddr->sin_addr.s_addr < m_Server.s_addr)  bCheck = true;
                                }
                                else
                                {
                                    bCheck = true;
                                }
                            }
                            else if(packet->iState == ESTATE_WAITING)
                            {
                                if(m_ServerPacket.iState == ESTATE_WAITING)
                                {
                                    if(packet->iSendCount > m_ServerPacket.iSendCount) bCheck = true;
                                    else if(pRemoteAddr->sin_addr.s_addr < m_Server.s_addr) bCheck = true;
                                }
                            }
                        }

                        if(bCheck)
                        {
                            memcpy(&m_ServerPacket, packet, sizeof(BroadCastPacket));
                            m_Server = pRemoteAddr->sin_addr;
                            m_bRecvServer = true;
                        }
                    }
                }
            }
        }
        else
        {
            OnRecv(event.pPacket);
        }
        break;

    case EEVENT_CLOSE:
    case EEVENT_ONSENDFAIL:
    case EEVENT_ONRECVFAIL:
        {
            int id = event.pSockObj->GetId();
            if(m_eState == ESTATE_CLIENT && id == ESOCKOBJTYPE_CLIENT)
            {
                SAFE_DELETESOCKOBJ(m_pSockObjs[ESOCKOBJTYPE_CLIENT]);
                ChangeState(ESTATE_WAITING);
                OnDisconnect();
            }
            else if(m_eState == ESTATE_HOST && id >= ESOCKOBJTYPE_COCLIENT_MIN && id < ESOCKOBJTYPE_COCLIENT_MAX)
            {
                SAFE_DELETESOCKOBJ(m_pSockObjs[id]);
                m_iCoClientNum--;
                if(m_iCoClientNum == 0)
                {
                    OnDisconnect();
                    ChangeState(ESTATE_WAITING);
                }
            }
        }
        break;

    case EEVENT_ONCONNECTFAIL:
        if(m_eState == ESTATE_CONNECTING)
        {
            SAFE_DELETESOCKOBJ(m_pSockObjs[ESOCKOBJTYPE_CLIENT]);
            ChangeState(ESTATE_WAITING);
        }
        break;

    case EEVENT_WRONGSOCKTYPE:
    case EEVENT_SOCKETALREADEXIST:
    case EEVENT_INVALIDSOCKET:
    case EEVENT_CREATEFAIL:
    case EEVENT_BINDFAIL:
    case EEVENT_LISTENFAIL:
    case EEVENT_POSTCONNECTFAIL:
    case EEVENT_POSTACCEPTFAIL:
    case EEVENT_POSTSENDFAIL:
    case EEVENT_POSTRECVFAIL:
    case EEVENT_ONACCEPTFAIL:
        return -1;
    }

    return NO_ERROR;
}

void CNetworkSystem::BroadCastCheckInfo()
{
    if(m_eState == ESTATE_WAITING && (m_fTime - m_fRecvServerTime > m_cBroadCastCheckServerInterval))
    {
        m_fRecvServerTime = m_fTime;
        if(m_bRecvServer && !m_pSockObjs[ESOCKOBJTYPE_CLIENT])
        {
            SOCKADDR_IN ServerAddr;

            ServerAddr.sin_family   = AF_INET;
            ServerAddr.sin_addr     = m_Server;
            ServerAddr.sin_port     = htons(MAIN_PORT);

            char buf[GAMENAME_LEN];
            strcpy_s(buf, GAMENAME_LEN, GAME_NAME);

            m_pSockObjs[ESOCKOBJTYPE_CLIENT] = CreateSockObj(ESOCKOBJTYPE_CLIENT);
            if(m_pSockObjs[ESOCKOBJTYPE_CLIENT]->Connect((SOCKADDR*)&ServerAddr, sizeof(ServerAddr), (SOCKADDR*)&m_LocalBindAddr
                , sizeof(m_LocalBindAddr), buf, GAMENAME_LEN) == NO_ERROR)
            {
                ChangeState(ESTATE_CONNECTING);
            }
            else
            {
                SAFE_DELETESOCKOBJ(m_pSockObjs[ESOCKOBJTYPE_CLIENT]);
                m_bRecvServer = false;
            }
        }
    }
}

int CNetworkSystem::BroadCastSendInfo()
{
    if(m_fTime - m_fBroadCastSendTime < m_cBroadCastSendInterval) return NO_ERROR;

    if((m_eState == ESTATE_WAITING) || (m_eState == ESTATE_HOST && m_iCoClientNum < m_iCoClientMax))
    {
        int rc;
        BroadCastPacket packet;
        strcpy_s(packet.sGameName, GAMENAME_LEN, GAME_NAME);
        packet.iSendCount       = m_iSendCount;
        packet.iState           = m_eState;
        packet.iCoClientNum     = m_iCoClientNum;
        m_fBroadCastSendTime    = m_fTime;
        m_iSendCount++;
        rc = m_pSockObjs[ESOCKOBJTYPE_BROADCAST]->Send(&packet, (SOCKADDR*)&m_BroadCastSendAddr, sizeof(m_BroadCastSendAddr));
        if(rc != NO_ERROR) return rc;
    }

    return NO_ERROR;
}

bool CNetworkSystem::FindEmptyCoClient(int& empty)
{
    for(UINT i = 0; i < m_iCoClientMax; i++)
    {
        if(!m_pSockObjs[ESOCKOBJTYPE_COCLIENT_MIN + i])
        {
            empty = i;
            return true;
        }
    }
    return false;
}

void CNetworkSystem::ChangeState(EState state)
{
    if(state == ESTATE_WAITING && m_eState != ESTATE_WAITING)
    {
        m_bRecvServer = false;
        m_fRecvServerTime = m_fTime;
    }
    m_eState = state;
}

ISockObj* CNetworkSystem::CreateSockObj(int id)
{
    return new CSockObj(id, this);
}

void CNetworkSystem::DeleteSockObj(ISockObj* obj)
{
    obj->Close();
    m_vectorDeletedSockObj.push_back(obj);
}

void CNetworkSystem::FreeDeletedSockObj()
{
    for(UINT i = 0; i < m_vectorDeletedSockObj.size(); i++)
    {
        SAFE_DELETE(m_vectorDeletedSockObj[i]);
    }
    m_vectorDeletedSockObj.clear();
}

void CNetworkSystem::OnAccept(int client)
{
    for(UINT i = 0; i < m_vectorListener.size(); i++)
    {
        m_vectorListener[i]->OnAccept(client);
    }
}

void CNetworkSystem::OnConnect()
{
    for(UINT i = 0; i < m_vectorListener.size(); i++)
    {
        m_vectorListener[i]->OnConnect();
    }
}

void CNetworkSystem::OnRecv(Packet* pPacket)
{
    for(UINT i = 0; i < m_vectorListener.size(); i++)
    {
        m_vectorListener[i]->OnRecv(pPacket);
    }
}

void CNetworkSystem::OnDisconnect()
{
    for(UINT i = 0; i < m_vectorListener.size(); i++)
    {
        m_vectorListener[i]->OnDisconnect();
    }
}

void CNetworkSystem::OnStop()
{
    for(UINT i = 0; i < m_vectorListener.size(); i++)
    {
        m_vectorListener[i]->OnStop();
    }
}
