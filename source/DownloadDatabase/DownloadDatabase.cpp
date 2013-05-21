//**************************************************
//File: DownloadDatabase.cpp
//Author: GaoJiongjiong
//Function: 下载关卡数据库
//**************************************************

#include "StdAfx.h"
#include "DownloadDatabase.h"

CDownloadDatabase::CDownloadDatabase(const char* proxy, int begin, int end, int con_num)
: m_iBegin(begin)
, m_iEnd(end)
, m_iConNum(con_num)
, m_iDispatchedNum(0)
, m_iDownloadedNum(0)
, m_ThreadHandles(NULL)
, m_pFile(NULL)
{
    if(proxy)
    {
        strcpy_s(m_Proxy, min(strlen(proxy), 32), proxy);
        m_bProxy = true;
    }
    else
    {
        m_bProxy = false;
    }
    m_iBegin    = begin;
    m_iEnd      = end;
    m_iConNum   = con_num;
}

int CDownloadDatabase::Run()
{
    int active_num, active_order, ret;
    int group, order;
    ThreadInfo *info;
    ThreadInfoMap::iterator it;

    printf("Download Percent:0.00%%\r");

    while(1)
    {
        DispatchTask();
        active_num = SortThread();
        //printf("active_num=%d\n", active_num);
        if(active_num <= 0) break;
        ret = WaitForMultipleObjects(active_num, m_ThreadHandles, false, INFINITE);
        active_order = ret - WAIT_OBJECT_0;
        if(active_order < 0 || active_order >= active_num) continue;
        it = m_mapThreadInfo.find(m_ThreadHandles[active_order]);
        if(it == m_mapThreadInfo.end() || !(it->second)) ERROR_RTN0("Can\'t find ThreadInfo in map!");
        info    = it->second;
        group   = info->group;
        order   = info->order;
        m_mapThreadInfo.erase(it);
        SAFE_CLOSEHANDLE(m_ThreadHandles[active_order]);
        SAFE_DELETE(info);
        if(m_Data[group][order].state != EBUFSTATE_DOWNLOADED) return 0;
        m_Data[group][order].state = EBUFSTATE_FINISH;
        m_iDownloadedNum++;
        printf("Download Percent:%.2f%%\r", m_iDownloadedNum * 100.0f / m_iPuzNum);
        if(group != m_iFrontGroup) continue;
        if(!IsAllFinish(m_iFrontGroup)) continue;
        for(int i = 0; i < m_iDataLen; i++)
        {
            if(m_Data[m_iFrontGroup][i].state == EBUFSTATE_FINISH)
            {
                fputs(m_Data[m_iFrontGroup][i].buf, m_pFile);
                fputs("\n", m_pFile);
            }
            m_Data[m_iFrontGroup][i].state = EBUFSTATE_UNDISPATCH;
        }
        m_iDataUsed[m_iFrontGroup]  = 0;
        m_iFrontGroup               = 1 - m_iFrontGroup;
        m_iBackGroup                = 1 - m_iBackGroup;
    }
    return 1;
}

int CDownloadDatabase::Init()
{
    m_iPuzNum           = m_iEnd - m_iBegin + 1;
    m_iConNum           = min(m_iConNum, m_iPuzNum);
    m_iDataLen          = m_iConNum * DATAMULT;
    m_ThreadHandles     = new HANDLE[m_iConNum];
    m_Data[0]           = new Data[m_iDataLen];
    m_Data[1]           = new Data[m_iDataLen];
    m_iDataUsed[0]      = 0;
    m_iDataUsed[1]      = 0;
    m_iFrontGroup       = 0;
    m_iBackGroup        = 1;

    for(int i = 0; i < m_iConNum; i++)
    {
        m_ThreadHandles[i] = NULL;
    }
    for(int i = 0; i < m_iDataLen; i++)
    {
        m_Data[0][i].state = EBUFSTATE_UNDISPATCH;
        m_Data[1][i].state = EBUFSTATE_UNDISPATCH;
    }

    char filename[32];
    sprintf_s(filename, 32, "%d-%d.txt", m_iBegin, m_iEnd);
    DeleteFile(filename);
    if(fopen_s(&m_pFile, filename, "w")) ERROR_RTN0("Can\'t open out file!");

    return 1;
}

void CDownloadDatabase::Release()
{
    SAFE_DELETEA(m_ThreadHandles);
    SAFE_DELETEA(m_Data[0]);
    SAFE_DELETEA(m_Data[1]);
    SAFE_FCLOSE(m_pFile);
}

void CDownloadDatabase::DispatchTask()
{
    for(int i = 0; i < m_iConNum; i++)
    {
        if(m_iDispatchedNum >= m_iPuzNum) break;
        if(m_ThreadHandles[i]) continue;

        ThreadInfo* info = new ThreadInfo;
        if(m_iDataUsed[m_iFrontGroup] < m_iDataLen)
        {
            info->group = m_iFrontGroup;
            info->order = m_iDataUsed[m_iFrontGroup]++;
        }
        else if(m_iDataUsed[m_iBackGroup] < m_iDataLen)
        {
            info->group = m_iBackGroup;
            info->order = m_iDataUsed[m_iBackGroup]++;
        }
        else
        {
            break;
        }

        Data* pData = &(m_Data[info->group][info->order]);
        pData->state = EBUFSTATE_DISPATCHED;
        info->data = pData;
        info->puzzle = m_iBegin + m_iDispatchedNum;
        m_iDispatchedNum++;
        if(m_bProxy) info->proxy = m_Proxy;
        else info->proxy = NULL;
        m_ThreadHandles[i] = CreateThread(NULL, 0, Download, info, 0, NULL);
        m_mapThreadInfo[m_ThreadHandles[i]] = info;
    }
}

int CDownloadDatabase::SortThread()
{
    int active_num = 0;
    int lastvalid = m_iConNum;
    for(int i = 0; i < lastvalid; i++)
    {
        if(m_ThreadHandles[i])
        {
            active_num++;
            continue;
        }

        for(int j = lastvalid - 1; j >= 0; j--)
        {
            lastvalid = j;
            if(m_ThreadHandles[j] == NULL) continue;
            break;
        }

        if(lastvalid <= i) break;
        SwapThread(i, lastvalid);
        active_num++;
    }

    return active_num;
}

void CDownloadDatabase::SwapThread(int i, int j)
{
    if(i < 0 || i > m_iConNum || j < 0 || j > m_iConNum) return;

    HANDLE handle = m_ThreadHandles[i];

    m_ThreadHandles[i] = m_ThreadHandles[j];
    m_ThreadHandles[j] = handle;
}

bool CDownloadDatabase::IsAllFinish(int group)
{
    if(group < 0 || group  >= 2) return false;
    for(int i = 0; i < m_iDataLen; i++)
    {
        if(m_Data[group][i].state == EBUFSTATE_FINISH) continue;
        if(m_iDispatchedNum >= m_iPuzNum && m_Data[group][i].state == EBUFSTATE_UNDISPATCH) continue;

        return false;
    }

    return true;
}

DWORD WINAPI CDownloadDatabase::Download(void* param)
{
    ThreadInfo* info = (ThreadInfo*)param;

    char temp_name[32];
    sprintf_s(temp_name, 32, "temp%d.txt", info->puzzle);

    char commond_buf[256];
    if(info->proxy)
    {
        sprintf_s(commond_buf, 256, "tool\\wget -e \"http_proxy = %s\" -q -O %s http://www.suduko.us/j/smallcn.php?xh=%d", info->proxy, temp_name, info->puzzle);
    }
    else
    {
        sprintf_s(commond_buf, 256, "tool\\wget -q -O %s http://www.suduko.us/j/smallcn.php?xh=%d", temp_name, info->puzzle);
    }

    //WinExec(commond_buf, SW_HIDE);
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb   = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcess(NULL, commond_buf, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

    WaitForSingleObject(pi.hProcess, INFINITE);

    FILE*   pfr = NULL;
    if(fopen_s(&pfr, temp_name, "r")) ERROR_RTN0("Can\'t download puzzle!");

    char    str_buf[BUFLEN];
    char    *p1, *p2;
    bool    bFound  = false;
    while(fgets(str_buf, BUFLEN, pfr))
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
    if(!bFound) ERROR_RTN0("Can\'t find puzzle!");

    strcpy_s(info->data->buf, min(strlen(p1), BUFLEN), p1);
    info->data->state = EBUFSTATE_DOWNLOADED;

    do 
    {
        Sleep(1000);
    }while(!DeleteFile(temp_name));

    return 1;
}