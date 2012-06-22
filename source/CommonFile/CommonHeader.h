#ifndef __COMMONHEADER_H__
#define __COMMONHEADER_H__

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

inline void InitRootDir()
{
    WCHAR   sFileName[_MAX_PATH];
    GetModuleFileNameW(GetModuleHandle(NULL), sFileName, sizeof(sFileName));
    WCHAR*  p   = StrStrIW(sFileName, L"\\Bin");
    if(p)   *p  = 0;
    SetCurrentDirectoryW(sFileName);
}

#endif // __COMMONHEADER_H__