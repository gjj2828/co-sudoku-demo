#ifndef __ITIMER_H__
#define __ITIMER_H__

class ITimer
{
public:
    virtual int Init() = 0;
    virtual float GetTime() = 0;
};

#endif // __ITIMER_H__