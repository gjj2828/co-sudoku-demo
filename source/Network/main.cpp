#include "StdAfx.h"
#include "NetworkSystem.h"

extern "C"
{
    DLL_EXPORT INetworkSystem* CreateNetworkSystem()
    {
        static char buffer[sizeof(CNetworkSystem)];
        return new ((void*)buffer) CNetworkSystem();
    }
};