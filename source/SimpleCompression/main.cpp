#include "StdAfx.h"
#include "SimpleCompression.h"

extern "C"
{
    DLL_EXPORT ICompression* CreateCompression()
    {
        static char buffer[sizeof(CSimpleCompression)];
        return new ((void*)buffer) CSimpleCompression();
    }
};