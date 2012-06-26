#ifndef __IGAME_H__
#define __IGAME_H__

class IGame
{
public:
    virtual bool    Init()      = 0;
    virtual void    Run()       = 0;
    virtual void    Release()   = 0;
};

#endif // __IGAME_H__