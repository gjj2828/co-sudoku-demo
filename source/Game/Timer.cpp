//**************************************************
//File: Timer.cpp
//Author: GaoJiongjiong
//Function: ”Œœ∑ ±º‰
//**************************************************

#include "StdAfx.h"
#include "Timer.h"

CPrecisionTimer::CPrecisionTimer()
: m_iFreq(0)
, m_iBase(0)
{
};

int CPrecisionTimer::Init()
{
    LARGE_INTEGER li;

    if(!QueryPerformanceFrequency(&li)) return 0;
    m_iFreq = li.QuadPart;

    QueryPerformanceCounter(&li);
    m_iBase = li.QuadPart;

    return 1;
}

float CPrecisionTimer::GetTime()
{
    LARGE_INTEGER now;

    QueryPerformanceCounter(&now);

    return (float)((double)(now.QuadPart - m_iBase) / (double)m_iFreq);
}

CNormalTimer::CNormalTimer()
: m_dwBase(0)
, m_dwLast(0)
, m_dwLoop(1)
{
};

int CNormalTimer::Init()
{
    m_dwBase = timeGetTime();
    m_dwLast = m_dwBase;

    return 1;
}

float CNormalTimer::GetTime()
{
    DWORD dwNow = timeGetTime();

    if(dwNow < m_dwLast) m_dwLoop++;

    m_dwLast = dwNow;

    return (float)((double)(m_dwLoop * m_cMilliSecondLoop + (dwNow - m_dwBase)) / (double)m_cMilliSecondFreq);
}