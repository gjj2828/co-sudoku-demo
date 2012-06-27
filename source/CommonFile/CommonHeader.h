#ifndef __COMMONHEADER_H__
#define __COMMONHEADER_H__

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include <shlwapi.h>
#include <new>

#pragma comment(lib, "shlwapi.lib")

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)

#define ERROR_MSG(context)  MessageBox(NULL, (context), "Error", MB_OK)
#define ERROR_EXIT(context) \
{                           \
    ERROR_MSG(context);     \
    exit(1);                \
}
#define ERROR_RTN0(context) \
{                           \
    ERROR_MSG(context);     \
    return 0;                \
}
inline void InitRootDir()
{
    WCHAR   sFileName[_MAX_PATH];
    GetModuleFileNameW(GetModuleHandle(NULL), sFileName, sizeof(sFileName));
    WCHAR*  p   = StrStrIW(sFileName, L"\\Bin");
    if(p)   *p  = 0;
    SetCurrentDirectoryW(sFileName);
}

#endif // __COMMONHEADER_H__