#ifndef __ISOCKOBJ_H__
#define __ISOCKOBJ_H__

#include <Packet.h>

class ISockObj
{
public:
    virtual int GetId()                 = 0;
    virtual int Accept()                = 0;
    virtual int Connect()               = 0;
    virtual int Send(Packet* packet)    = 0;
    virtual int Recv()                  = 0;
};

#endif // __ISOCKOBJ_H__