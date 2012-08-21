#include "StdAfx.h"
#include "NetworkSystem.h"

CNetworkSystem::CNetworkSystem(): m_eState(ESTATE_STOPPED)
{
}

const char* CNetworkSystem::m_cBroadCastAddr = "192.168.255.255";

int CNetworkSystem::Init()
{
    WSADATA wsd;

    if(WSAStartup(MAKEWORD(2, 2), &wsd)) ERROR_RTN0("Can\'t load Winsock!");

    m_BroadCastBindAddr.sin_family      = AF_INET;
    m_BroadCastBindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_BroadCastBindAddr.sin_port        = htons(BROADCAST_PORT);

    m_BroadCastAddr.sin_family          = AF_INET;
    m_BroadCastAddr.sin_addr.s_addr     = inet_addr(m_cBroadCastAddr);
    m_BroadCastAddr.sin_port            = htons(BROADCAST_PORT);

    m_BindAddr.sin_family               = AF_INET;
    m_BindAddr.sin_addr.s_addr          = htonl(INADDR_ANY);
    m_BindAddr.sin_port                 = htons(MAIN_PORT);

    m_soBroadCast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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
}

void CNetworkSystem::Start(EMode mode, float time)
{
    if(m_eState != ESTATE_STOPPED) return;

    m_fTimeBase = time;
}