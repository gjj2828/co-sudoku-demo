#include "StdAfx.h"
#include "CompressionSystem.h"

extern "C"
{
    DLL_EXPORT ICompressionSystem* CreateCompressionSystem()
    {
        static char buffer[sizeof(CCompressionSystem)];
        return new ((void*)buffer) CCompressionSystem();
    }
};