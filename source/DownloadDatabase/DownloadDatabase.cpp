#include "StdAfx.h"
#include "DownloadDatabase.h"

CDownloadDatabase::CDownloadDatabase(const char* proxy, int begin, int end)
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
    m_iBegin = begin;
    m_iEnd = end;
}