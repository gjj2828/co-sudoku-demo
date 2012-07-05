#include "StdAfx.h"
#include "DownloadDatabase.h"

CDownloadDatabase::CDownloadDatabase(const char* proxy, int begin, int end, int con_num)
{
    if(proxy)
    {
        strcpy(m_Proxy, proxy);
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
    int active_num;
    while(1)
    {
        DispatchTask();
        active_num = SortThread();
        if(active_num <= 0) break;
    }
    return 1;
}

void CDownloadDatabase::Init()
{
    m_iPuzNum           = m_iEnd - m_iBegin + 1;
    m_iConNum           = min(m_iConNum, m_iPuzNum);
    m_iActiveConNum     = m_iConNum;
    m_iDownloadedNum    = 0;
    m_iDataLen          = m_iConNum * DATAMULT;
    m_pThreadHandles    = new HANDLE[m_iConNum];
    m_pThreadInfo       = new ThreadInfo[m_iConNum];
    m_pData[0]          = new Data[m_iDataLen];
    m_pData[1]          = new Data[m_iDataLen];
    m_iDataUsed[0]      = 0;
    m_iDataUsed[1]      = 0;
    m_iFrontGroup       = 0;
    m_iBackGroup        = 1;

    for(int i = 0; i < m_iConNum; i++)
    {
        m_pThreadHandles[i] = NULL;
    }
    for(int i = 0; i < m_iDataLen; i++)
    {
        m_pData[0][i].state = EBUFSTATE_UNDOWNLOAD;
        m_pData[1][i].state = EBUFSTATE_UNDOWNLOAD;
    }
}

void CDownloadDatabase::Release()
{
    delete[] m_pThreadHandles;
    delete[] m_pThreadInfo;
    delete[] m_pData[0];
    delete[] m_pData[1];
}

void CDownloadDatabase::DispatchTask()
{
    for(int i = 0; i < m_iConNum; i++)
    {
        if(m_iDownloadedNum >= m_iPuzNum) break;
        if(m_pThreadHandles[i]) continue;

        if(m_iDataUsed[m_iFrontGroup] < m_iDataLen)
        {
            m_pThreadInfo[i].group = m_iFrontGroup;
            m_pThreadInfo[i].order = m_iDataUsed[m_iFrontGroup]++;
        }
        else if(m_iDataUsed[m_iBackGroup] < m_iDataLen)
        {
            m_pThreadInfo[i].group = m_iBackGroup;
            m_pThreadInfo[i].order = m_iDataUsed[m_iBackGroup]++;
        }
        else
        {
            break;
        }

        m_pThreadInfo[i].data = &(m_pData[m_pThreadInfo[i].group][m_pThreadInfo[i].order]);
        m_pThreadInfo[i].puzzle = m_iBegin + m_iDownloadedNum;
        m_iDownloadedNum++;
        m_pThreadHandles[i] = CreateThread(NULL, 0, Download, &(m_pThreadInfo[i]), 0, NULL);
    }
}

int CDownloadDatabase::SortThread()
{
    int active_num = 0;
    int lastvalid = m_iConNum;
    for(int i = 0; i < lastvalid; i++)
    {
        if(m_pThreadHandles[i])
        {
            active_num++;
            continue;
        }

        for(int j = lastvalid - 1; j >= 0; j--)
        {
            if(m_pThreadHandles[j] == NULL) continue;
            lastvalid = j;
            break;
        }

        if(lastvalid <= i) break;
        SwapThread(i, lastvalid);
    }

    return active_num;
}

void CDownloadDatabase::SwapThread(int i, int j)
{
    if(i < 0 || i > m_iConNum || j < 0 || j > m_iConNum) return;

    HANDLE handle = m_pThreadHandles[i];
    ThreadInfo info = m_pThreadInfo[i];

    m_pThreadHandles[i] = m_pThreadHandles[j];
    m_pThreadInfo[i]    = m_pThreadInfo[j];
    m_pThreadHandles[j] = handle;
    m_pThreadInfo[j]    = info;
}

DWORD WINAPI CDownloadDatabase::Download(void* param)
{
    return 0;
}