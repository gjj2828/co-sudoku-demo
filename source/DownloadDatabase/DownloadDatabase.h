#ifndef __DOWNLOADDATABASE_H__
#define __DOWNLOADDATABASE_H__

#include "IDownloadDatabase.h"

class CDownloadDatabase: public IDownloadDatabase
{
public:
    CDownloadDatabase(const char* proxy, int begin, int end, int con_num);

    virtual int Run();

private:
    enum
    {
        BUFLEN = 256,
        DATAMULT = 2,
    };
    enum EBufState
    {
        EBUFSTATE_MIN,
        EBUFSTATE_UNDOWNLOAD = EBUFSTATE_MIN,
        EBUFSTATE_DOWNLOADING,
        EBUFSTATE_DOWNLOADED,
        EBUFSTATE_DOWNLOADERROR,
        EBUFSTATE_MAX,
    };

    struct Data
    {
        char        buf[BUFLEN];
        EBufState   state;
    };

    struct ThreadInfo
    {
        //HANDLE  thread;
        int     group;
        int     order;
        int     puzzle;
        Data*   data;
    };

    bool            m_bProxy;
    char            m_Proxy[32];
    int             m_iBegin;
    int             m_iEnd;
    int             m_iConNum;
    int             m_iActiveConNum;
    int             m_iPuzNum;
    int             m_iDownloadedNum;
    HANDLE*         m_pThreadHandles;
    ThreadInfo*     m_pThreadInfo;
    Data*           m_pData[2];
    int             m_iDataLen;
    int             m_iDataUsed[2];
    int             m_iFrontGroup;
    int             m_iBackGroup;

    void    Init();
    void    Release();
    void    DispatchTask();
    int     SortThread();
    void    SwapThread(int i, int j);

    static DWORD WINAPI Download(void* param);
};

#endif // __DOWNLOADDATABASE_H__