#ifndef __COMMONHEADER_H__
#define __COMMONHEADER_H__

#include <windows.h>
#include <WinDef.h>
#include <stdio.h>
#include <io.h>
#include <sys/stat.h>
#include <shlwapi.h>
#include <new>
#include <bitset>
#include <queue>
#include <map>

#pragma comment(lib, "shlwapi.lib")

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)

enum
{
    ECOMPRESSION_MIN,
    ECOMPRESSION_SIMPLE = ECOMPRESSION_MIN,
    ECOMPRESSION_MAX,
};

enum
{
    EPUZZLEGRADE_MIN,
    EPUZZLEGRADE_EASY = EPUZZLEGRADE_MIN,
    EPUZZLEGRADE_GENTLE,
    EPUZZLEGRADE_MODERATE,
    EPUZZLEGRADE_TOUGH,
    EPUZZLEGRADE_DIABOLICAL,
    EPUZZLEGRADE_MAX,
};

#define SAFE_FCLOSE(p)  \
{                       \
    if(p) fclose(p);    \
    (p) = NULL;         \
}

#define SAFE_FINDCLOSE(p)       \
{                               \
    if(p != -1) _findclose(p);  \
    (p) = NULL;                 \
}

#define SAFE_RELEASE(p)     \
{                           \
    if(p) (p)->Release();   \
    (p) = NULL;             \
}

#define SAFE_DELETE(p)  \
{                       \
    if(p) delete (p);   \
    (p) = NULL;         \
}

#define SAFE_DELETEA(p)  \
{                       \
    if(p) delete[] (p);   \
    (p) = NULL;         \
}

#define SAFE_FREELIBRARY(p) \
{                           \
    if(p) FreeLibrary(p);   \
    (p) = NULL;             \
}

#define SAFE_CLOSEHANDLE(p) \
{                           \
    if(p) CloseHandle(p);   \
    (p) = NULL;             \
}

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

#define BITTOCHARBUFFER(bit, buffer, bitlen, buflen)            \
{                                                               \
    int index = 0;                                              \
    for(int i = 0; i < (buflen) && index < (bitlen); i++)       \
    {                                                           \
        for(int j = 0; j < CHAR_BIT && index < (bitlen); j++)   \
        {                                                       \
            UCHAR a      = (bit)[index];                        \
            (buffer)[i] |= (a << j);                            \
            index++;                                            \
        }                                                       \
    }                                                           \
}

#define CHARBUFFERTOBIT(buffer, bit, buflen, bitlen)            \
{                                                               \
    int index = 0;                                              \
    for(int i = 0; i < (buflen) && index < (bitlen); i++)       \
    {                                                           \
        for(int j = 0; j < CHAR_BIT && index < (bitlen); j++)   \
        {                                                       \
            (bit)[index] = (buffer)[i] & (1 << j);              \
            index++;                                            \
        }                                                       \
    }                                                           \
}

inline void InitRootDir()
{
    WCHAR   sFileName[_MAX_PATH];
    GetModuleFileNameW(GetModuleHandle(NULL), sFileName, sizeof(sFileName));
    WCHAR*  p   = StrRStrIW(sFileName, NULL, L"\\Bin\\");
    if(p)   *p  = 0;
    SetCurrentDirectoryW(sFileName);
}

#include <ComPuzExpr.h>

#endif // __COMMONHEADER_H__