#ifndef __DOWNLOADDATABASE_H__
#define __DOWNLOADDATABASE_H__

#include "IDownloadDatabase.h"

class CDownloadDatabase: public IDownloadDatabase
{
public:
    CDownloadDatabase(const char* proxy, int begin, int end, int con_num);

    virtual int Init();
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
        EBUFSTATE_UNDISPATCH = EBUFSTATE_MIN,
        EBUFSTATE_DISPATCHED,
        EBUFSTATE_DOWNLOADED,
        EBUFSTATE_FINISH,
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
        char*   proxy;
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
    int             m_iPuzNum;
    int             m_iDispatchedNum;
    int             m_iDownloadedNum;
    HANDLE*         m_ThreadHandles;
    ThreadInfo*     m_ThreadInfo;
    Data*           m_Data[2];
    int             m_iDataLen;
    int             m_iDataUsed[2];
    int             m_iFrontGroup;
    int             m_iBackGroup;
    FILE*           m_pFile;

    void    Release();
    void    DispatchTask();
    int     SortThread();
    void    SwapThread(int i, int j);
    bool    IsAllFinish(int group);

    static DWORD WINAPI Download(void* param);
};

#endif // __DOWNLOADDATABASE_H__