#include "StdAfx.h"

#define ERROR_MSG(context)  MessageBox(NULL, (context), "Error", MB_OK)
#define ID_MIN  (1)
#define ID_MAX  (50000)

void    main()
{
    InitRootDir();

    bool    bProxy;
    char    proxy[64];
    printf("Do you need proxy?[y/n]:");
    char    buf[16];
    if(!scanf("%s", buf))
    {
        ERROR_MSG("Wrong Input!");
        return;
    }
    if(strcmp(buf, "y") == 0)
    {
        bProxy  = true;
    }
    else if(strcmp(buf, "n") == 0)
    {
        bProxy  = false;
    }
    else
    {
        ERROR_MSG("Wrong Input!");
        return;
    }
    if(bProxy)
    {
        printf("Please input proxy:");
        if(!scanf("%s", proxy))
        {
            ERROR_MSG("Wrong Input!");
            return;
        }
    }
    printf("Please input the download range[1-50000]:\n");
    int iBegin, iEnd;
    printf("begin=");
    if(!scanf("%d", &iBegin) || iBegin < ID_MIN || iBegin > ID_MAX)
    {
        ERROR_MSG("Wrong Input!");
        return;
    }
    printf("end=");
    if(!scanf("%d", &iEnd) || iEnd < iBegin || iEnd > ID_MAX)
    {
        ERROR_MSG("Wrong Input!");
        return;
    }
    char    OutName[32];
    sprintf(OutName, "%d-%d.txt", iBegin, iEnd);
    DeleteFile(OutName);
    int iNum    = iEnd - iBegin + 1;
    printf("Download Percent:%%0\r");
    for(int i = 0; i < iNum; i++)
    {
        int     iPuzzle     = iBegin + i;
        char*   TempName    = "temp.txt";
        char    CommondBuf[256];
        if(bProxy)
        {
            sprintf(CommondBuf, "tool\\wget -e \"http_proxy = %s\" -q -O %s http://www.suduko.us/j/smallcn.php?xh=%d", proxy, TempName, iPuzzle);
        }
        else
        {
            sprintf(CommondBuf, "tool\\wget -q -O %s http://www.suduko.us/j/smallcn.php?xh=%d", TempName, iPuzzle);
        }
        //WinExec(CommondBuf, SW_HIDE);
        STARTUPINFO         si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb   = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        CreateProcess(NULL, CommondBuf, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

        WaitForSingleObject(pi.hProcess, INFINITE);

        FILE*   pfr   = fopen(TempName, "r");
        if(!pfr)
        {
            ERROR_MSG("Can\'t download puzzle!");
            return;
        }
        char    str_buf[200];
        char    *p1, *p2;
        bool    bFound  = false;
        while(fgets(str_buf, 200, pfr))
        {
            p1  = StrStr(str_buf, "tmda=\'");
            if(!p1) continue;
            p1  += 6;
            p2  = StrStr(p1, "\'");
            if(!p2) break;
            *p2 = 0;
            bFound  = true;
            break;
        }
        fclose(pfr);
        DeleteFile(TempName);
        if(!bFound)
        {
            ERROR_MSG("Can\'t find puzzle!");
            return;
        }
        FILE*   pfw = fopen(OutName, "a");
        if(!pfw)
        {
            ERROR_MSG("Can\'t open out file!");
            return;
        }
        fputs(p1, pfw);
        fputs("\n", pfw);
        fclose(pfw);
        printf("Download Percent:%%%d\r", (i + 1) * 100 / iNum);
        Sleep(1000);
    }
}