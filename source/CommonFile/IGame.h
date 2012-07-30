#ifndef __IGAME_H__
#define __IGAME_H__

#include <GlobalEnviroment.h>

class IGame
{
public:
    virtual int                 Init(HINSTANCE hInstance)   = 0;
    virtual void                Run()                       = 0;
    virtual void                Release()                   = 0;
    virtual GlobalEnviroment*   GetEnv()                    = 0;
    virtual void                Paint()                     = 0;
    virtual void                MouseMove(int x, int y)     = 0;
};

#endif // __IGAME_H__