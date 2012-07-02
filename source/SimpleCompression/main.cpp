#include "StdAfx.h"
#include "SimpleCompression.h"

extern "C"
{
    DLL_EXPORT ICompression* GetCompression()
    {
        static CSimpleCompression compression;
        return &compression;
    }
};