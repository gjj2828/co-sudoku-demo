#ifndef __DOWNLOADDATABASE_H__
#define __DOWNLOADDATABASE_H__

#include "IDownloadDatabase.h"

class CDownloadDatabase: public IDownloadDatabase
{
public:
    CDownloadDatabase(const char* proxy, int begin, int end);

    virtual int Run() {return 1;}

private:
    bool    m_bProxy;
    char    m_Proxy[32];
    int     m_iBegin;
    int     m_iEnd;
};

#endif // __DOWNLOADDATABASE_H__