#include "StdAfx.h"
#include "NetworkSystem.h"

CNetworkSystem::CNetworkSystem(): m_eState(ESTATE_STOPPED)
{
}

int CNetworkSystem::Init()
{
    WSADATA wsd;

    if(WSAStartup(MAKEWORD(2, 2), &wsd)) ERROR_RTN0("Can\'t load Winsock!");

    return 1;
}

void CNetworkSystem::Release()
{
    WSACleanup();

    this->~CNetworkSystem();
}

void CNetworkSystem::Update()
{
    if(m_eState == ESTATE_STOPPED) return;
}

void CNetworkSystem::Start(EMode mode)
{
    if(m_eState != ESTATE_STOPPED) return;
}