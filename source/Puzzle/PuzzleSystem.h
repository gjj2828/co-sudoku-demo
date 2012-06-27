#ifndef __PUZZLESYSTEM_H__
#define __PUZZLESYSTEM_H__

#include <IPuzzleSystem.h>

class CPuzzleSystem: public IPuzzleSystem
{
public:
    virtual int     Init()      {return 1;}
    virtual void    Release()   {this->~CPuzzleSystem();}
};

#endif // __PUZZLESYSTEM_H__