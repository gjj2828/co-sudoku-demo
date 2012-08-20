#ifndef __TIMER_H__
#define __TIMER_H__

#include "ITimer.h"

class CPrecisionTimer: public ITimer
{
public:
    CPrecisionTimer();
    virtual int Init();
    virtual float GetTime();

private:
    int64 m_iFreq;
    int64 m_iBase;
};


class CNormalTimer: public ITimer
{
public:
    CNormalTimer();
    virtual int Init();
    virtual float GetTime();

private:
    DWORD m_dwBase;
    DWORD m_dwLast;
    DWORD m_dwLoop;

    static const int64 m_cMilliSecondLoop = 0x100000000;
    static const DWORD m_cMilliSecondFreq = 1000;
};

#endif // __TIMER_H__